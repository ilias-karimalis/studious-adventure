/*
	Linker script for the RISC-v QEMU "virt" machine detailed at:
	https://github.com/qemu/qemu/blob/master/hw/riscv/virt.c -
*/
OUTPUT_ARCH("riscv")

ENTRY(start)

MEMORY
{
	ram (wxa!ri) : ORIGIN = 0x80000000, LENGTH = 4096M /* 4GB RAM */
}

PHDRS
{
	text PT_LOAD;
	data PT_LOAD;
	bss  PT_LOAD;
}

SECTIONS
{
	/* Kernel code */
	.text : {
		. = ALIGN(4096);
		PROVIDE(_text_start = .);
		*(.text.init) *(.text .text.*)
		PROVIDE(_text_end = .);
	} >ram AT>ram :text

	PROVIDE(_global_pointer = .);

	/* Read only data */
	.rodata : {
		. = ALIGN(4096);
		PROVIDE(_rodata_start = .);
		*(.rodata .rodata.*)
		PROVIDE(_rodata_end = .);
	} >ram AT>ram :text

	.data : {
		. = ALIGN(4096);
		PROVIDE(_data_start = .);
		*(.sdata .sdata.*) *(.data .data.*)
		PROVIDE(_data_end = .);
	} >ram AT>ram :data

	.bss : {
		. = ALIGN(4096);
		PROVIDE(_bss_start = .);
		*(.sbss .sbss.*) *(.bss .bss.*)
		PROVIDE(_bss_end = .);
	} >ram AT>ram :bss

	PROVIDE(_memory_start = ORIGIN(ram));

	. = ALIGN(4096);
	PROVIDE(_stack_start = .);
	. = . + 0x8000;
	PROVIDE(_stack_end = .);

	PROVIDE(_memory_end = ORIGIN(ram) + LENGTH(ram));

	PROVIDE(_heap_start = _stack_end);
	PROVIDE(_heap_end = _memory_end);
	PROVIDE(_heap_size = _memory_end - _heap_start);
}
