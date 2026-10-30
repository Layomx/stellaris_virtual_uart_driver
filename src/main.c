/*
    UART Driver demo.

    1. Configure UART0
    2. Print a banner and the live values of the UART registers.
    3. Send three periodic "tick" messages.
    4. Echo every received byte.
*/

#include "LM3S6965_regs.h"
#include "uart.h"
#include <stdint.h>

// QEMU's lm3s6965evb model starts with a 12.5 MHz system clock.
#define SYSCLK_HZ 12500000u
#define CTRL_D 0x04u
#define TICK_COUNT 3u
#define DELAY_ITERATIONS 20000000u

static int print_hex32(uint32_t value) {
  static const char digits[] = "0123456789ABCDEF";

  if (uart_write("0x") != UART_OK) {
    return UART_ERR_TIMEOUT;
  }
  for (int shift = 20; shift >= 0; shift -= 4) {
    if (uart_write_byte((uint8_t)digits[(value >> shift) & 0xFu]) != UART_OK) {
      return UART_ERR_TIMEOUT;
    }
  }
  return UART_OK;
}

static int print_reg(const char *name, uint32_t base, uint32_t offset) {
  if (uart_write(name) != UART_OK || uart_write(" = ") != UART_OK ||
      print_hex32(*reg(base, offset)) != UART_OK ||
      uart_write("\r\n") != UART_OK) {
    return UART_ERR_TIMEOUT;
  }
  return UART_OK;
}

static int print_flags(uint32_t fr) {
  if (uart_write("  FR flags:") != UART_OK) {
    return UART_ERR_TIMEOUT;
  }
  if (fr & UART_FR_TXFE) {
    if (uart_write(" TXFE") != UART_OK)
      return UART_ERR_TIMEOUT;
  }
  if (fr & UART_FR_TXFF) {
    if (uart_write(" TXFF") != UART_OK)
      return UART_ERR_TIMEOUT;
  }
  if (fr & UART_FR_RXFE) {
    if (uart_write(" RXFE") != UART_OK)
      return UART_ERR_TIMEOUT;
  }
  if (fr & UART_FR_RXFF) {
    if (uart_write(" RXFF") != UART_OK)
      return UART_ERR_TIMEOUT;
  }
  if (fr & UART_FR_BUSY) {
    if (uart_write(" BUSY") != UART_OK)
      return UART_ERR_TIMEOUT;
  }
  return uart_write("\r\n");
}

static int dump_state(void) {
  if (uart_write("--- register dump ---\r\n") != UART_OK ||
      print_reg("SYSCTL RCC   ", SYSCTL_BASE, SYSCTL_RCC_OFFSET) != UART_OK ||
      print_reg("SYSCTL RCGC1 ", SYSCTL_BASE, SYSCTL_RCGC1_OFFSET) != UART_OK ||
      print_reg("SYSCTL RCGC2 ", SYSCTL_BASE, SYSCTL_RCGC2_OFFSET) != UART_OK ||
      print_reg("GPIOA AFSEL  ", GPIO_BASE, GPIO_AFSEL_OFFSET) != UART_OK ||
      print_reg("GPIOA DEN    ", GPIO_BASE, GPIO_DEN_OFFSET) != UART_OK ||
      print_reg("UART0 IBRD   ", UART0_BASE, UART_IBRD_OFFSET) != UART_OK ||
      print_reg("UART0 FBRD   ", UART0_BASE, UART_FBRD_OFFSET) != UART_OK ||
      print_reg("UART0 LCRH   ", UART0_BASE, UART_LCRH_OFFSET) != UART_OK ||
      print_reg("UART0 CTL    ", UART0_BASE, UART_CTL_OFFSET) != UART_OK ||
      print_reg("UART0 FR     ", UART0_BASE, UART_FR_OFFSET) != UART_OK ||
      print_flags(*reg(UART0_BASE, UART_FR_OFFSET)) != UART_OK) {
    return UART_ERR_TIMEOUT;
  }
  return UART_OK;
}

static void delay(void) {
  for (volatile uint32_t i = 0; i < DELAY_ITERATIONS; i++) {
  }
}

int main(void) {
  const uart_config_t cfg = {
      .baud = 115200u,
      .data_bits = 8u,
      .parity = UART_PARITY_NONE,
      .stop_bits = 1u,
      .use_fifo = true,
  };

  if (uart_init(&cfg, SYSCLK_HZ) != UART_OK) {
    for (;;) {
    }
  }

  if (uart_write("\r\n === stellaris-uart-driver (LM3S6965 / QEMU) ===\r\n") !=
          UART_OK ||
      dump_state() != UART_OK) {
    return 1;
  }

  for (uint32_t n = 1; n <= TICK_COUNT; n++) {
    if (uart_write("[tick ") != UART_OK ||
        uart_write_byte((uint8_t)('0' + n)) != UART_OK ||
        uart_write("]\r\n") != UART_OK) {
      return 1;
    }
    delay();
  }

  if (uart_write("Echo mode: type text, '?' = registers, Ctrl-D = stop\r\n") !=
      UART_OK) {
    return 1;
  }
  for (;;) {
    uint8_t ch;
    uint32_t err;
    int status = uart_read_byte(&ch, &err);

    if (status == UART_ERR_TIMEOUT) {
      continue;
    }
    if (status != UART_OK) {
      return 1;
    }
    if (err != 0u) {
      if (uart_write("[rx error flags ") != UART_OK ||
          print_hex32(err) != UART_OK || uart_write("]\r\n") != UART_OK) {
        return 1;
      }
      continue;
    }
    if (ch == CTRL_D) {
      break;
    }
    if (ch == '?') {
      if (uart_write("\r\n") != UART_OK || dump_state() != UART_OK) {
        return 1;
      }
    } else if (ch == '\r') {
      if (uart_write("\r\n") != UART_OK) {
        return 1;
      }
    } else {
      if (uart_write_byte(ch) != UART_OK) {
        return 1;
      }
    }
  }

  if (uart_write("\r\nBye.\r\n") != UART_OK || uart_flush() != UART_OK) {
    return 1;
  }
  for (;;) {
    __asm__ volatile("wfi");
  }
}
