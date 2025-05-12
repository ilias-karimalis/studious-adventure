.section .text.init

.option norvc

.type start, @function
.global start
start:
	.cfi_startproc
	
.option push
.option norelax
	la gp, _global_pointer
.option pop

	# Any hardware threads (hart) that are not bootstrapping need to wait for an IPI
	# to be sent from the bootstrapping hart.
	csrr t0, mhartid
	bnez t0, wait

	
	/* Reset satp */
	csrw satp, zero
	
	/* Setup stack */
	la sp, _stack_start
	
	/* Clear the BSS section */
	la t5, _bss_start
	la t6, _bss_end
bss_clear:
	sd zero, (t5)
	addi t5, t5, 8
	bltu t5, t6, bss_clear
	
	la t0, kmain
	csrw mepc, t0
	
	/* Jump to kernel! */
	tail kmain

wait:
	/* Wait for an IPI to be sent from the bootstrapping hart */
	wfi
	j wait
	
	.cfi_endproc

.end
