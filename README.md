# Stellaris UART

Bare-metal firmware project for the QEMU `lm3s6965evb` virtual board.

The target is a Cortex-M3-based Stellaris LM3S6965 environment. The project
uses a custom linker script, a Cortex-M3 startup file, and the
`arm-none-eabi` GCC toolchain.

## Requirements

- CMake 3.20 or newer
- `arm-none-eabi-gcc`
- `arm-none-eabi-objcopy`
- `arm-none-eabi-size`
- QEMU with ARM system emulation
- clangd (optional, for editor integration)

## Configure and build

```sh
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake
cmake --build build
```

Build artifacts are generated inside `build/`:

- `stellaris_uart.elf`: ELF image for debugging.
- `stellaris_uart.bin`: raw binary firmware image.
- `stellaris_uart.map`: linker map file.
- `compile_commands.json`: compilation database for tools such as clangd.

## Run in QEMU

```sh
cmake --build build --target qemu
```

The QEMU target runs the firmware using the `lm3s6965evb` machine and
connects UART output to the terminal.

## clangd integration

The `.clangd` file points clangd to the compilation database in `build/`.
After configuring the project, clangd uses the same compiler flags as the
firmware build, including:

- `-mcpu=cortex-m3`
- `-mthumb`
- `-ffreestanding`
