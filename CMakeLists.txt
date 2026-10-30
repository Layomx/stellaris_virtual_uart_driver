cmake_minimum_required(VERSION 3.20)

project(stellaris_uart LANGUAGES C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(FIRMWARE_TARGET stellaris_uart)
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/ld/LM3S6965.ld)

add_executable(${FIRMWARE_TARGET}
    src/main.c
    src/startup.c
    src/uart.c
)

target_include_directories(${FIRMWARE_TARGET}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_compile_options(${FIRMWARE_TARGET} PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -ffreestanding
    -fdata-sections
    -ffunction-sections
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

target_link_options(${FIRMWARE_TARGET} PRIVATE
    -mcpu=cortex-m3
    -mthumb
    -T${LINKER_SCRIPT}
    -nostdlib
    -nostartfiles
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE_TARGET}.map
    -Wl,--print-memory-usage
)

target_link_libraries(${FIRMWARE_TARGET} PRIVATE gcc)

set_target_properties(${FIRMWARE_TARGET} PROPERTIES
    SUFFIX ".elf"
)

find_program(ARM_OBJCOPY arm-none-eabi-objcopy REQUIRED)
find_program(ARM_SIZE arm-none-eabi-size REQUIRED)

add_custom_command(TARGET ${FIRMWARE_TARGET} POST_BUILD
    COMMAND ${ARM_OBJCOPY} -O binary
            $<TARGET_FILE:${FIRMWARE_TARGET}>
            ${CMAKE_CURRENT_BINARY_DIR}/${FIRMWARE_TARGET}.bin
    COMMAND ${ARM_SIZE} $<TARGET_FILE:${FIRMWARE_TARGET}>
    COMMENT "Generating ${FIRMWARE_TARGET}.bin"
)

add_custom_target(qemu
    COMMAND qemu-system-arm
            -M lm3s6965evb
            -kernel $<TARGET_FILE:${FIRMWARE_TARGET}>
            -nographic
            -serial mon:stdio
    DEPENDS ${FIRMWARE_TARGET}
    USES_TERMINAL
    COMMENT "Running firmware in QEMU"
)

add_custom_target(qemu-test
    COMMAND ${CMAKE_SOURCE_DIR}/tests/run_qemu.sh
            $<TARGET_FILE:${FIRMWARE_TARGET}>
    DEPENDS ${FIRMWARE_TARGET}
    USES_TERMINAL
    COMMENT "Running the QEMU smoke test"
)
