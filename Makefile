CC=riscv64-unknown-elf-gcc
AS=riscv64-unknown-elf-as
LD=riscv64-unknown-elf-ld
CFLAGS=-Wall -Wextra -c -mcmodel=medany -ffreestanding -nostdlib -fno-exceptions -std=c18
INCLUDES=-I./include
LINKER_SCRIPT=linker.ld
OUT=os.elf
SRC_ASM=$(wildcard kernel/asm/*.S)
SRC_C=$(wildcard kernel/*.c)
OBJ_ASM=$(SRC_ASM:.S=.o)
OBJ_C=$(SRC_C:.c=.o)
OBJS=$(OBJ_ASM) $(OBJ_C)

# QEMU Options
QEMU=qemu-system-riscv64
MACH=virt
CPU=rv64
CPUS=2
MEMORY=128M

all: $(OUT)

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

# Compile Assembly files
%.o: %.S
	$(AS) $(INCLUDES) -o $@ $<

# Link all object files
$(OUT): $(OBJS)
	$(LD) -T $(LINKER_SCRIPT) -o $@ $(OBJS)

# Run QEMU
run: all
	$(QEMU) -M $(MACH) -cpu $(CPU) -smp $(CPUS) -m $(MEMORY) -nographic -serial mon:stdio -bios none -kernel $(OUT)

clean: 
	rm -f $(OUT) $(OBJS)
