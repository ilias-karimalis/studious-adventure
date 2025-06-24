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
    parser.add_argument("--kernel", "-k", required=True, help="Path to the kernel ELF file.")

    group = parser.add_mutually_exclusive_group()
    group.add_argument("--gdb", default=False, action=argparse.BooleanOptionalAction,
                    help="Runs QEMU in the background and launches GDB for debugging.")
    group.add_argument("--dump-dtb", default=False, action=argparse.BooleanOptionalAction,
                   help="Dumps the DTB file of the virt machine, with the current kernel to a virt.dtb file.")

    args = parser.parse_args()

    qemu_args = [
        QEMU_BINARY,
        "-M", "virt",
        "-cpu", "rv64",
        "-smp", "2",
        "-m", "4G",
        "-nographic",
        "-serial", "mon:stdio",
        "-bios", "none",
        "-kernel", args.kernel,
    ]
    if args.dump_dtb:
        qemu_args += ["-machine", "dumpdtb=virt.dtb"]
        print("Dumping DTB file to virt.dtb")
        try:
            os.execvp(qemu_args[0], qemu_args)
        except OSError as e:
            print(f"Error launching QEMU to dump DTB: {e}")
            sys.exit(1)
    elif args.gdb:
        print("GDB Script is not working, currently just prints out the two commands to run.")
        # Add flags for QEMU to wait for GDB
        qemu_args += ["-s", "-S"]  # -s = -gdb tcp::1234, -S = freeze CPU at startup
        print(' '.join(qemu_args))

        gdb_commands = [
            GDB_BINARY,
            args.kernel,
            "-ex", "\"set architecture riscv:rv64\"",
            "-ex", "\"target remote localhost:1234\"",
        ]
        print(' '.join(gdb_commands))
    else:
        # Launch QEMU replacing this current process
        print("Preparing to launch QEMU.")
        print("QEMU Path: ", QEMU_BINARY)
        print("QEMU Arguments: ", ' '.join(qemu_args[1:]))
        try:
            os.execvp(qemu_args[0], qemu_args)
        except OSError as e:
            print(f"Error launching QEMU: {e}")
            sys.exit(1)

if __name__ == "__main__":
    main()
