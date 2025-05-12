# riscv64-toolchain.cmake

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

# Define compilers
set(CMAKE_C_COMPILER riscv64-unknown-elf-gcc)
set(CMAKE_ASM_COMPILER riscv64-unknown-elf-as)
set(CMAKE_LINKER riscv64-unknown-elf-ld)

# Tell CMake not to look for standard libraries
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_ASM_COMPILER_WORKS TRUE)

# Optional: Disable standard toolchain file behavior
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
