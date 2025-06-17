#include <kernel/uart_ns16550a.h>
#include <kernel/pmm.h>
#include <kernel/paging.h>

#include <grouperlib/arch/riscv.h>
#include <grouperlib/fmtprint.h>
#include <grouperlib/error.h>
#include <grouperlib/bitmacros.h>

/// ID maps a range of virtual addresses to physical addresses in the kernel's page table.
void kernel_id_map_range(sv39_pageTable *root, paddr_t start, paddr_t end, u64 flags)
{
	vaddr_t aligned_start = ALIGN_DOWN(start, BASE_PAGE_SIZE);
	vaddr_t aligned_end = ALIGN_UP(end, BASE_PAGE_SIZE);
	ASSERT(aligned_start < aligned_end, "Start address must be less than end address");
	print("[kernel_id_map_range] Mapping range: %x to %x with flags: %x\n", aligned_start, aligned_end, flags);

	for (paddr_t pa = aligned_start; pa < aligned_end; pa += BASE_PAGE_SIZE) {
		errval_t err = sv39_map(root, pa, pa, flags, sv39_Page);
		if (err_is_fail(err)) {
			ASSERT(false, "[kernel_id_map_range] Failed to map address %x to %x: %s", pa, pa, err_str(err));
		}
	}
}

size_t kinit(void)
{
	errval_t err;

	uart_ns16550a_initialize(UART_NS16550A_BASE);

	// Initialize the string library with the UART functions
	strlib_initialize(uart_ns16550a_putchar);

	// Print the greeting visual
	print("                    _,,......_                           \n");
	print("                 ,-'          `'--.                      \n");
	print("              ,-'  _              '-.                    \n");
	print("     (`.    ,'   ,  `-.              `.                  \n");
	print("      \\ \\  -    / )    \\               \\             \n");
	print("       `\\`-^^^, )/      |     /         :               \n");
	print("         )^ ^ ^V/            /          '.               \n");
	print("         |      )            |           `.              \n");
	print("         9   9 /,--,\\    |._:`         .._`.            \n");
	print("         |    /   /  `.  \\    `.      (   `.`.          \n");
	print("         |   / \\  \\    \\  \\     `--\\   )    `.`.__  \n");
	print("-hrr-   .;;./  '   )   '   )       ///'       `-\"       \n");
	print("        `--'   7//\\    ///\\                            \n");
	print("                                                         \n");
	print("=========================================================\n");
	print("\tBooting Ilias' toy OS                                  \n");
	print("=========================================================\n");

	print("[kinit] uart NS16550A initialized @ %x.\n", UART_NS16550A_BASE);

	print("[kinit] Global Values:\n");
	print("\t* Heap Start:               %x\n", HEAP_START);
	print("\t* Heap Size:                %x\n", HEAP_SIZE);
	print("\t* Text Start:               %x\n", TEXT_START);
	print("\t* Text End:                 %x\n", TEXT_END);
	print("\t* Data Start:               %x\n", DATA_START);
	print("\t* Data End:                 %x\n", DATA_END);
	print("\t* RoData Start:             %x\n", RODATA_START);
	print("\t* RoData End:               %x\n", RODATA_END);
	print("\t* Bss Start:                %x\n", BSS_START);
	print("\t* Bss End:                  %x\n", BSS_END);
	print("\t* Kernel Stack Start:       %x\n", STACK_START);
	print("\t* Kernel Stack End:         %x\n", STACK_END);

	// Initialize the physical memory manager
	err = pmm_initialize();
	if (err_is_fail(err)) {
		PANIC_LOOP("[kinit] Failed to initialize pmm: %s\n", err_str(err));
	}
	print("[kinit] Empty pmm initialized.\n");
	err = pmm_add_region((u8 *)HEAP_START, (size_t)HEAP_SIZE);
	if (err_is_fail(err)) {
		PANIC_LOOP("[kinit] Failed to add initial pmm region: %s\n", err_str(err));
	}
	print("[kinit] pmm initialized with %x bytes of memory.\n", pmm_total_mem());

	// Initialize kernel paging
	sv39_pageTable *root = (sv39_pageTable*) sv39_paging_init();
	// Setting up kernel identity mappings
	kernel_id_map_range(root, TEXT_START, TEXT_END, SV39_FLAGS_READ | SV39_FLAGS_EXECUTE);
	kernel_id_map_range(root, RODATA_START, RODATA_END, SV39_FLAGS_READ);
	kernel_id_map_range(root, DATA_START, DATA_END, SV39_FLAGS_READ | SV39_FLAGS_WRITE);
	kernel_id_map_range(root, BSS_START, BSS_END, SV39_FLAGS_READ | SV39_FLAGS_WRITE);
	kernel_id_map_range(root, STACK_START, STACK_END, SV39_FLAGS_READ | SV39_FLAGS_WRITE);
	kernel_id_map_range(root, HEAP_START, HEAP_END, SV39_FLAGS_READ | SV39_FLAGS_WRITE);
	kernel_id_map_range(root, UART_NS16550A_BASE, UART_NS16550A_BASE + BASE_PAGE_SIZE,
			    SV39_FLAGS_READ | SV39_FLAGS_WRITE);
	print("[kinit] Kernel paging initialized.\n");

	// Assert that identity mappings are correct!
	for (vaddr_t va = TEXT_START; va < TEXT_END; va += BASE_PAGE_SIZE) {
		OPT(paddr_t) pa = sv39_virt_to_phys(root, va);
		ASSERT(OPT_EQ(pa, va), "Identity mapping failed, mapping valid: %d, va: %x, pa: %x\n", pa.some, va,
		       pa.val);
	}
	for (vaddr_t va = RODATA_START; va < RODATA_END; va += BASE_PAGE_SIZE) {
		OPT(paddr_t) pa = sv39_virt_to_phys(root, va);
		ASSERT(OPT_EQ(pa, va), "RODATA: Identity mapping failed, mapping valid: %d, va: %lx, pa: %lx\n",
		       pa.some, va, pa.val);
	}
	for (vaddr_t va = DATA_START; va < DATA_END; va += BASE_PAGE_SIZE) {
		OPT(paddr_t) pa = sv39_virt_to_phys(root, va);
		ASSERT(OPT_EQ(pa, va), "DATA: Identity mapping failed, mapping valid: %d, va: %lx, pa: %lx\n", pa.some,
		       va, pa.val);
	}
	for (vaddr_t va = BSS_START; va < BSS_END; va += BASE_PAGE_SIZE) {
		OPT(paddr_t) pa = sv39_virt_to_phys(root, va);
		ASSERT(OPT_EQ(pa, va), "BSS: Identity mapping failed, mapping valid: %d, va: %lx, pa: %lx\n", pa.some,
		       va, pa.val);
	}
	for (vaddr_t va = STACK_START; va < STACK_END; va += BASE_PAGE_SIZE) {
		OPT(paddr_t) pa = sv39_virt_to_phys(root, va);
		ASSERT(OPT_EQ(pa, va), "STACK: Identity mapping failed, mapping valid: %d, va: %lx, pa: %lx\n", pa.some,
		       va, pa.val);
	}
	for (vaddr_t va = HEAP_START; va < HEAP_END; va += BASE_PAGE_SIZE) {
		OPT(paddr_t) pa = sv39_virt_to_phys(root, va);
		ASSERT(OPT_EQ(pa, va), "HEAP: Identity mapping failed, mapping valid: %d, va: %lx, pa: %lx\n", pa.some,
		       va, pa.val);
	}
	for (vaddr_t va = UART_NS16550A_BASE; va < UART_NS16550A_BASE + BASE_PAGE_SIZE; va += BASE_PAGE_SIZE) {
		OPT(paddr_t) pa = sv39_virt_to_phys(root, va);
		ASSERT(OPT_EQ(pa, va), "UART: Identity mapping failed, mapping valid: %d, va: %lx, pa: %lx\n", pa.some,
		       va, pa.val);
	}
	// Returns the value to write to the SATP register
	return SATP_MODE_SV39_FLAG | SATP_PPN_MASK(root);
}

/// Note: The value of sepc (address of `kmain`) needs to have it's lower to bits be zeroed
__attribute__((aligned(4))) void kmain(void)
{
	print("[kmain] Paging enabled. Kernel is now running with paging.\n");

	// Main loop of the kernel
	while (1) {
		// Here you would typically handle interrupts, system calls, etc.
	}
}