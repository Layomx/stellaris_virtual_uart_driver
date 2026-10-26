/*
    UART Driver demo.

    1. Configure UART0
    2. Print a banner and the live values of the UART registers.
    3. Send three periodic "tick" messages.
    4. Echo every received byte.
*/

#include "uart.h"
#include <LM3S6965_regs.h>
#include <stdint.h>

// RCC resets to the 6 MHz, PLL bypassed, no divider
#define SYSCLK_HZ 60000000u
#define CTRL_D 0x04u
#define TICK_COUNT 3u
#define DELAY_ITERATIONS 20000000u

static void print_hex32(uint32_t value) {
  static const char digits[] = "0123456789ABCDEF";

  uart_write("0x");
  for (int shift = 20; shift >= 0; shift -= 4) {
    uart_write_byte((uint8_t)digits[(value >> shift) & 0xFu]);
  }
}

static void print_reg(const char *name, uint32_t base, uint32_t offset) {
  uart_write(name);
  uart_write(" = ");
  print_hex32(*reg(base, offset));
  uart_write("\r\n");
}

static void print_flags(uint32_t fr) {
  uart_write("  FR flags:");
  if (fr & UART_FR_TXFE) {
    uart_write(" TXFE");
  }
  if (fr & UART_FR_TXFF) {
    uart_write(" TXFF");
  }
  if (fr & UART_FR_RXFE) {
    uart_write(" RXFE");
  }
  if (fr & UART_FR_RXFF) {
    uart_write(" RXFF");
  }
  if (fr & UART_FR_BUSY) {
    uart_write(" BUSY");
  }
}

static void dump_state(void) {
  uart_write("--- register dump ---\r\n");
  print_reg("SYSCTL RCC   ", SYSCTL_BASE, SYSCTL_RCC_OFFSET);
  print_reg("SYSCTL RCGC1 ", SYSCTL_BASE, SYSCTL_RCGC1_OFFSET);
  print_reg("SYSCTL RCGC2 ", SYSCTL_BASE, SYSCTL_RCGC2_OFFSET);
  print_reg("GPIOA AFSEL  ", GPIO_BASE, GPIO_AFSEL_OFFSET);
  print_reg("GPIOA DEN    ", GPIO_BASE, GPIO_DEN_OFFSET);
  print_reg("UART0 IBRD   ", UART0_BASE, UART_IBRD_OFFSET);
  print_reg("UART0 FBRD   ", UART0_BASE, UART_FBRD_OFFSET);
  print_reg("UART0 LCRH   ", UART0_BASE, UART_LCRH_OFFSET);
  print_reg("UART0 CTL    ", UART0_BASE, UART_CTL_OFFSET);
  print_reg("UART0 FR     ", UART0_BASE, UART_FR_OFFSET);
  print_flags(*reg(UART0_BASE, UART_FR_OFFSET));
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

  uart_write("\r\n === stellaris-uart-driver (LMS3S6965 / QEMU) ===\r\n");
  dump_state();

  for (uint32_t n = 1; n <= TICK_COUNT; n++) {
    uart_write("[tick ");
    uart_write_byte((uint8_t)('0' + n));
    uart_write("]\r\n");
    delay();
  }

  uart_write("Echo mode: type text, '?' = registers, Ctrl-D = stop\r\n");
  for (;;) {
    uint8_t ch;
    uint32_t err = uart_read_byte(&ch);

    if (err != 0u) {
      uart_write("[rx error flags ");
      print_hex32(err);
      uart_write("]\r\n");
      continue;
    }
    if (ch == CTRL_D) {
      break;
    }
    if (ch == '?') {
      uart_write("\r\n");
      dump_state();
    } else if (ch == '\r') {
      uart_write("\r\n");
    } else {
      uart_write_byte(ch);
    }
  }

  uart_write("\r\nBye.\r\n");
  uart_flush();
  for (;;) {
    __asm__ volatile("wfi");
  }
}
