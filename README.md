# Stellaris UART

Bare-metal UART0 firmware and driver for the TI Stellaris LM3S6965EVB model
in QEMU. The project is intentionally small: it provides a production-minded
starting point for register-level Cortex-M3 firmware without a HAL, CMSIS,
RTOS, or libc.

## Current scope

The firmware:

- Configures UART0 on PA0 (RX) and PA1 (TX).
- Supports 5–8 data bits, one or two stop bits, odd/even/no parity, and
  optional hardware FIFOs.
- Uses polling with bounded iteration-based timeouts.
- Prints register state and periodic messages.
- Enters an interactive echo mode.
- Prints the received UART error flags.

The default clock is the QEMU LM3S6965 reset clock of 12.5 MHz. The clock
value is passed explicitly to `uart_init`; firmware using a different clock
must pass the actual frequency.

## Requirements

- CMake 3.20 or newer
- Ninja or another CMake generator
- `gcc-arm-none-eabi`
- `binutils-arm-none-eabi`
- `qemu-system-arm`
- `clangd` (optional)

On Debian or Ubuntu:

```sh
sudo apt-get install gcc-arm-none-eabi binutils-arm-none-eabi \
  cmake ninja-build qemu-system-arm clangd
```

## Configure and build

```sh
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake
cmake --build build --parallel
```

Generated files:

- `build/stellaris_uart.elf`: ELF image for QEMU and GDB.
- `build/stellaris_uart.bin`: raw firmware image.
- `build/stellaris_uart.map`: linker map.
- `build/compile_commands.json`: clangd compilation database.

Run the QEMU smoke test:

```sh
cmake --build build --target qemu-test
```

Run the firmware interactively:

```sh
cmake --build build --target qemu
```

QEMU uses the `lm3s6965evb` machine and connects UART output to the terminal.
Exit with `Ctrl-A`, then `x`.

## Interactive behavior

After startup and three tick messages, the firmware accepts input:

- Ordinary characters are echoed.
- `?` prints the UART and clock register state.
- `Ctrl-D` exits the echo loop and prints `Bye.`.

The QEMU smoke test only verifies startup output; it does not inject
interactive input.

## Project layout

- `include/LM3S6965_regs.h`: memory-mapped register definitions.
- `include/uart.h`: public UART API and status codes.
- `src/uart.c`: UART0 initialization and polling driver.
- `src/main.c`: demo application.
- `src/startup.c`: vector table and C runtime initialization.
- `ld/LM3S6965.ld`: Flash/RAM memory layout.
- `cmake/arm-none-eabi.cmake`: cross-compilation toolchain.
- `tests/run_qemu.sh`: non-interactive QEMU smoke test.

## Driver API

`uart_init()` validates the configuration and programs the baud-rate
divisors, pin mux, line control, FIFO, and UART enable bits. The data path
uses:

```c
int uart_write_byte(uint8_t byte);
int uart_write(const char *str);
int uart_flush(void);
int uart_read_byte(uint8_t *byte, uint32_t *errors);
```

These functions return `UART_OK`, `UART_ERR_PARAM`, or
`UART_ERR_TIMEOUT`. `uart_read_byte()` reports framing, parity, break, and
overrun flags separately through `errors`.

## Production status and remaining work

This is a robust educational boilerplate, not production-certified hardware
support. Before using it in a product, the following work is still required:

- Validate the register map and pin mux against the exact LM3S6965 silicon and
  board revision.
- Validate baud rate, electrical levels, clock tolerance, and signal integrity
  on physical hardware with a scope or logic analyzer.
- Replace iteration-based timeouts with a calibrated time source or timer.
- Decide the concurrency model: interrupt-driven RX/TX, DMA, or a documented
  polling-only integration contract.
- Add a receive buffer and an explicit policy for FIFO overrun.
- Define reset, fault, and watchdog behavior.
- Add unit tests for baud divisor rounding and configuration validation.
- Add integration tests for parity, framing, break, overrun, and timeout paths.
- Review compiler warnings, linker map, stack usage, and worst-case latency for
  the target product.
- Add a hardware-in-the-loop test before claiming board-level support.

QEMU validates the startup path and visible UART behavior, but it does not
replace electrical or timing validation on the real board.
