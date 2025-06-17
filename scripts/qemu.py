#!/usr/bin/python3

# This script handles all the QEMU-related tasks, including running GDB for debugging.

import argparse
import subprocess
import os
import sys
import time

QEMU_BINARY = "qemu-system-riscv64"
GDB_BINARY = "gdb-multiarch"


def main():
    parser = argparse.ArgumentParser(description="Runs QEMU with a given riscv64 kernel on the virt machine.")
    parser.add_argument("--kernel-elf", required=True, help="Path to the kernel ELF file.")
    parser.add_argument("--gdb", default=False, action=argparse.BooleanOptionalAction,
                        help="Runs QEMU in the background and launches GDB for debugging.")
    args = parser.parse_args()

    qemu_args = [
        QEMU_BINARY,
        "-M", "virt",
        "-cpu", "rv64",
        "-smp", "2",
        "-m", "128M",
        "-nographic",
        "-serial", "mon:stdio",
        "-bios", "none",
        "-kernel", args.kernel_elf
    ]
    if not args.gdb:
        # Launch QEMU replacing this current process
        print("Preparing to launch QEMU.")
        print("QEMU Path: ", QEMU_BINARY)
        print("QEMU Arguments: ", ' '.join(qemu_args[1:]))
        try:
            os.execvp(qemu_args[0], qemu_args)
        except OSError as e:
            print(f"Error launching QEMU: {e}")
            sys.exit(1)
    else:
        # Add flags for QEMU to wait for GDB
        qemu_args += ["-s", "-S"]  # -s = -gdb tcp::1234, -S = freeze CPU at startup

        print("Launching QEMU in the background for GDB debugging...")
        print("QEMU Command: ", ' '.join(qemu_args))
        try:
            now = time.time()
            qemu_stdout = open(f"qemu_stdout_{now}.log", "w")
            qemu_stderr = open(f"qemu_stderr_{now}.log", "w")
            qemu_process = subprocess.Popen(qemu_args, stdout=qemu_stdout, stderr=qemu_stderr, text=True)
        except Exception as e:
            print(f"Failed to start QEMU: {e}")
            sys.exit(1)

        # Allow some time for QEMU to start and listen on port 1234
        time.sleep(1)
        gdb_commands = [
            GDB_BINARY,
            args.kernel_elf,
            "-ex", "set architecture riscv:rv64",
            "-ex", "target remote localhost:1234",
            "-ex", "layout asm",
            "-ex", "layout regs",
        ]
        print("Preparing to launch GDB.")
        print("GDB Command: ", ' '.join(gdb_commands))
        try:
            os.execvp(gdb_commands[0], gdb_commands)
        except OSError as e:
            print(f"Error launching GDB: {e}")
            qemu_process.terminate()
            qemu_process.wait()
            sys.exit(1)


if __name__ == "__main__":
    main()
