#include "LM3S6965_regs.h"
#include "uart.h"
#include <stddef.h>

/*
 Baud-rate divisor, from the datasheet:  BRD = sysclk / (16 * baud)
 IBRD = integer part, FBRD = round(fractional part * 64).
 Doing it in fixed point: div64 = round(64 * BRD) = round(4 * sysclk / baud).
 (4 * sysclk) fits in 32 bits for sysclk < ~1 GHz.
 */
static int compute_divisors(uint32_t sysclk_hz, uint32_t baud, uint32_t *ibrd,
                            uint32_t *fbrd) {
  if (baud == 0u || sysclk_hz < 16u * baud) {
    return UART_ERR_PARAM;
  }
  uint32_t div64 = (4u * sysclk_hz + baud / 2u) / baud;
  *ibrd = div64 >> 6;
  *fbrd = div64 & 0x3Fu;
  if (*ibrd == 0u || *ibrd > 0xFFFFu) {
    return UART_ERR_PARAM;
  }
  return UART_OK;
}

static uint32_t build_lcrh(const uart_config_t *cfg) {
  uint32_t lcrh = (uint32_t)(cfg->data_bits - 5u) << UART_LCRH_WLEN_SHIFT;

  if (cfg->parity != UART_PARITY_NONE) {
    lcrh |= UART_LCRH_PEN;
    if (cfg->parity == UART_PARITY_EVEN) {
      lcrh |= UART_LCRH_EPS;
    }
  }
  if (cfg->stop_bits == 2u) {
    lcrh |= UART_LCRH_STP2;
  }
  if (cfg->use_fifo) {
    lcrh |= UART_LCRH_FEN;
  }
  return lcrh;
}

int uart_init(const uart_config_t *cfg, uint32_t sysclk_hz) {
  uint32_t ibrd, fbrd;

  if (cfg == NULL || cfg->baud == 0u ||
      cfg->data_bits < 5u || cfg->data_bits > 8u ||
      (cfg->parity != UART_PARITY_NONE &&
       cfg->parity != UART_PARITY_ODD &&
       cfg->parity != UART_PARITY_EVEN) ||
      (cfg->stop_bits != 1u && cfg->stop_bits != 2u)) {
    return UART_ERR_PARAM;
  }
  if (compute_divisors(sysclk_hz, cfg->baud, &ibrd, &fbrd) != UART_OK) {
    return UART_ERR_PARAM;
  }

  // Clock gating: UART0 and GPIOA must be clocked before being touched.
  *reg(SYSCTL_BASE, SYSCTL_RCGC1_OFFSET) |= SYSCTL_RCGC1_UART0;
  *reg(SYSCTL_BASE, SYSCTL_RCGC2_OFFSET) |= SYSCTL_RCGC2_GPIOA;
  /* The datasheet requires a few cycles before the first access; a
   * read-back of the register just written provides that delay. */
  (void)*reg(SYSCTL_BASE, SYSCTL_RCGC2_OFFSET);

  /* 2. Pin mux: hand PA0/PA1 to UART0 and enable their digital path. */
  const uint32_t uart0_pins = GPIOA_PIN_U0RX | GPIOA_PIN_U0TX;
  *reg(GPIO_BASE, GPIO_AFSEL_OFFSET) |= uart0_pins;
  *reg(GPIO_BASE, GPIO_DEN_OFFSET) |= uart0_pins;

  uint32_t pctl = *reg(GPIO_BASE, GPIO_PCTL_OFFSET);
  pctl &= ~(GPIO_PCTL_PA0_MASK | GPIO_PCTL_PA1_MASK);
  pctl |= GPIO_PCTL_PA0_U0RX | GPIO_PCTL_PA1_U0TX;
  *reg(GPIO_BASE, GPIO_PCTL_OFFSET) = pctl;

  // Disable the UART while it is being reconfigured.
  *reg(UART0_BASE, UART_CTL_OFFSET) &= ~UART_CTL_UARTEN;

  /* 4. Baud rate, then line control. The datasheet requires IBRD/FBRD to be
   * written before LCRH, because the LCRH write latches the divisor. */
  *reg(UART0_BASE, UART_IBRD_OFFSET) = ibrd;
  *reg(UART0_BASE, UART_FBRD_OFFSET) = fbrd;
  *reg(UART0_BASE, UART_LCRH_OFFSET) = build_lcrh(cfg);

  // Enable transmitter, receiver and the UART itself.
  *reg(UART0_BASE, UART_CTL_OFFSET) =
      UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE;

  return UART_OK;
}

bool uart_tx_ready(void) {
  return (*reg(UART0_BASE, UART_FR_OFFSET) & UART_FR_TXFF) == 0u;
}

bool uart_rx_ready(void) {
  return (*reg(UART0_BASE, UART_FR_OFFSET) & UART_FR_RXFE) == 0u;
}

bool uart_tx_busy(void) {
  return (*reg(UART0_BASE, UART_FR_OFFSET) & UART_FR_BUSY) != 0u;
}

void uart_write_byte(uint8_t byte) {
  while (!uart_tx_ready()) {
    // TXFF set: FIFO full (or the single holding register is occupied).
  }
  *reg(UART0_BASE, UART_DR_OFFSET) = byte;
}

void uart_write(const char *str) {
  while (*str != '\0') {
    uart_write_byte((uint8_t)*str++);
  }
}

void uart_flush(void) {
  while (uart_tx_busy()) {
  }
}

uint32_t uart_read_byte(uint8_t *byte) {
  while (!uart_rx_ready()) {
    // RXFE set: nothing received yet.
  }
  uint32_t dr = *reg(UART0_BASE, UART_DR_OFFSET); // one read pops the FIFO
  *byte = (uint8_t)(dr & UART_DR_DATA_MASK);
  return (dr >> UART_DR_ERR_SHIFT) & UART_DR_ERR_MASK;
}
