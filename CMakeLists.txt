cmake_minimum_required(VERSION 3.16)
project(riscv_kernel C ASM)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(ENABLE_ASSERTIONS)
    message(STATUS "Assertions enabled (Debug build)")
endif()

# Specify cross-compiler tools (defined in the toolchain file)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Make sure to generate the compile_commands file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define the linker script
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker.ld)

# Collect sources
file(GLOB_RECURSE SRC_C src/*.c)
file(GLOB_RECURSE SRC_ASM src/*.S)

set(SOURCES ${SRC_C} ${SRC_ASM})

# Define the executable
add_executable(${PROJECT_NAME}.elf ${SOURCES})

# Include headers
target_include_directories(${PROJECT_NAME}.elf PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Compiler flags
target_compile_options(${PROJECT_NAME}.elf PRIVATE
        -Wall -Wextra -Werror -mcmodel=medany -ffreestanding -nostdlib -fno-exceptions
)

# Linker flags
target_link_options(${PROJECT_NAME}.elf PRIVATE
        -T${LINKER_SCRIPT} -nostdlib -ffreestanding
)

# Custom command to run in QEMU
add_custom_target(run
        COMMAND  ${CMAKE_SOURCE_DIR}/scripts/qemu.py --kernel ${PROJECT_NAME}.elf
        DEPENDS ${PROJECT_NAME}.elf
        COMMENT "Running in QEMU..."
)
