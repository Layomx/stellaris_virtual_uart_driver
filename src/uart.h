#ifndef UART_H
#define UART_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  UART_PARITY_NONE,
  UART_PARITY_ODD,
  UART_PARITY_EVEN
} uart_parity_t;

typedef struct {
  uint32_t baud;     // bits per second
  uint8_t data_bits; // 5..8
  uart_parity_t parity;
  uint8_t stop_bits; // 1 or 2
  bool use_fifo;     // enable the 16 entry TX/RX FIFOs
} uart_config_t;

// Receive error flags, same bit positions as UARTDR[11:8] shifted down
#define UART_RX_ERR_FRAMING (1u << 0)
#define UART_RX_ERR_PARITY (1u << 1)
#define UART_RX_ERR_BREAK (1u << 2)
#define UART_RX_ERR_OVERRUN (1u << 3)

#define UART_OK 0
#define UART_ERR_PARAM (-1)

// Configure UART0 on PA0/PA1. sysclk_hz is the UART input clck (system clock)
int uart_init(const uart_config_t *cfg, uint32_t sysclk_hz);

bool uart_tx_ready(void); // true is a byte can be written now
bool uart_rx_ready(void); // true if a byte is waiting to be read
bool uart_tx_busy(void);  // true while bits are still being shifted

void uart_write_byte(uint8_t byte); // blocks while TX FIFO is full
void uart_write(const char *str);   // NUL terminated string
void uart_flush(void);              // blocks until the line is idle

// Blocks until a byte arrives. Returns 0 or a mask of UART_RX_ERR_* flags
uint32_t uart_read_byte(uint8_t *byte);

#endif
