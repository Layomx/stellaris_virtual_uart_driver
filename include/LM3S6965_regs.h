/*
    Register map for the peripherals used by this project.

    Source: Stellaris LM3S6965 Microcontroller datasheet (memory map + register chapters).
*/

#ifndef LM3S6965_REGS_H
#define LM3S6965_REGS_H

#include <stdint.h>

// System control
#define SYSCTL_BASE             0x400FE000u

#define SYSCTL_RCC_OFFSET       0x060u // Run-Mode clock config
#define SYSCTL_RCGC1_OFFSET     0x104u // Run-Mode clock gating, group 1
#define SYSCTL_RCGC2_OFFSET     0x108u // Run-Mode clock gating, group 2

#define SYSCTL_RCGC1_UART0 (1u << 0)
#define SYSCTL_RCGC2_GPIOA (1u << 0)

// GPIO Port A (APB)
#define GPIO_BASE         0x40004000u
#define GPIO_AFSEL_OFFSET 0x420u // Alternate Function Select
#define GPIO_DEN_OFFSET   0x51Cu // Digital Enable
#define GPIO_PCTL_OFFSET  0x52Cu

#define GPIOA_PIN_U0RX (1u << 0) // PA0
#define GPIOA_PIN_U0TX (1u << 1) // PA1

// GPIOPCTL function 1 selects UART0 on PA0 and PA1.
#define GPIO_PCTL_PA0_MASK (0xFu << 0)
#define GPIO_PCTL_PA1_MASK (0xFu << 4)
#define GPIO_PCTL_PA0_U0RX (0x1u << 0)
#define GPIO_PCTL_PA1_U0TX (0x1u << 4)

// UART0
#define UART0_BASE        0x4000C000u
#define UART_DR_OFFSET    0x000u // Data
#define UART_FR_OFFSET    0x018u // Flag
#define UART_IBRD_OFFSET  0x024u // Integer Baud-Rate Divisor
#define UART_FBRD_OFFSET  0x028u // Fractional Baud-Rate Divisor
#define UART_LCRH_OFFSET  0x02Cu // Line Control
#define UART_CTL_OFFSET   0x030u // Control

// UARTDR, read side: bits 11:8 carry error flags for the byte in bits 7:0
#define UART_DR_DATA_MASK 0x000000FFu
#define UART_DR_ERR_SHIFT 8u
#define UART_DR_ERR_MASK  0x0Fu
#define UART_DR_ERR_BITS  0x00000F00u

// UARTFR
#define UART_FR_BUSY (1u << 3) // shifter still transmitting
#define UART_FR_RXFE (1u << 4) // RX FIFO empty
#define UART_FR_TXFF (1u << 5) // TX FIFO full
#define UART_FR_RXFF (1u << 6) // RX FIFO full
#define UART_FR_TXFE (1u << 7) // TX FIFO empty

// UARTLCRH
#define UART_LCRH_PEN (1u << 1)  // parity enable
#define UART_LCRH_EPS (1u << 2)  // even parity select
#define UART_LCRH_STP2 (1u << 3) // two stop bits
#define UART_LCRH_FEN  (1u << 4) // FIFO enable
#define UART_LCRH_WLEN_SHIFT 5u  // 0=5 bits ... 3=8 bits
#define UART_LCRH_WLEN_8 (3u << UART_LCRH_WLEN_SHIFT)

// UARTCTL
#define UART_CTL_UARTEN (1u << 0)
#define UART_CTL_TXE    (1u << 8)
#define UART_CTL_RXE    (1u << 9)

/*
    Memory-mapped register access

    The returned pointer is volatile uint32_t: every read and write through it is a real bus access that the compiler must preserve.
*/
static inline volatile uint32_t *reg(uint32_t base, uint32_t offset)
{
    return (volatile uint32_t *)(uintptr_t)(base + offset);
}

#endif
