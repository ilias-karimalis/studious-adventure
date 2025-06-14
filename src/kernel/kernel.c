#include <kernel/uart_ns16550a.h>
#include <kernel/pmm.h>
#include <kernel/paging.h>

#include <grouperlib/arch/riscv.h>
#include <grouperlib/fmtprint.h>
#include <grouperlib/error.h>
#include <grouperlib/bitmacros.h>

/// ID maps a range of virtual addresses to physical addresses in the kernel's page table.
void kernel_id_map_range(sv39_pageTable* root, paddr_t start, paddr_t end, u64 flags)
{
	vaddr_t aligned_start = ALIGN_DOWN(start,  BASE_PAGE_SIZE);
	vaddr_t aligned_end = ALIGN_UP(end, BASE_PAGE_SIZE);
	ASSERT(aligned_start < aligned_end, "Start address must be less than end address");
	fmtprint("[kernel_id_map_range] Mapping range: %x to %x with flags: %x\n", aligned_start, aligned_end, flags);

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

	fmtprint("[kinit] uart NS16550A initialized @ %x.\n", UART_NS16550A_BASE);

	fmtprint("[kinit] Global Values:\n");
	fmtprint("\t* Heap Start:               %x\n", HEAP_START);
	fmtprint("\t* Heap Size:                %x\n", HEAP_SIZE);
	fmtprint("\t* Text Start:               %x\n", TEXT_START);
	fmtprint("\t* Text End:                 %x\n", TEXT_END);
	fmtprint("\t* Data Start:               %x\n", DATA_START);
	fmtprint("\t* Data End:                 %x\n", DATA_END);
	fmtprint("\t* RoData Start:             %x\n", RODATA_START);
	fmtprint("\t* RoData End:               %x\n", RODATA_END);
	fmtprint("\t* Bss Start:                %x\n", BSS_START);
	fmtprint("\t* Bss End:                  %x\n", BSS_END);
	fmtprint("\t* Kernel Stack Start:       %x\n", STACK_START);
	fmtprint("\t* Kernel Stack End:         %x\n", STACK_END);

	// Initialize the physical memory manager
	err = pmm_initialize();
	if (err_is_fail(err)) {
		fmtprint("[kinit] Failed to initialize pmm: %s\n", err_str(err));
		return;
	}
	fmtprint("[kinit] Empty pmm initialized.\n");
	err = pmm_add_region((u8*)HEAP_START, (size_t)HEAP_SIZE);
	if (err_is_fail(err)) {
		fmtprint("[kinit] Failed to add initial pmm region: %s\n", err_str(err));
		return;
	}
	fmtprint("[kinit] pmm initialized with %x bytes of memory.\n", pmm_total_mem());

	// Initialize kernel paging
	sv39_pageTable* root = sv39_paging_init();
	// Setting up kernel identity mappings
	kernel_id_map_range(root, TEXT_START, TEXT_END, 	SV39_FLAGS_READ | SV39_FLAGS_EXECUTE);
	kernel_id_map_range(root, RODATA_START, RODATA_END, SV39_FLAGS_READ);
	kernel_id_map_range(root, DATA_START, DATA_END, 	SV39_FLAGS_READ | SV39_FLAGS_WRITE);
	kernel_id_map_range(root, BSS_START, BSS_END, 		SV39_FLAGS_READ | SV39_FLAGS_WRITE);
	kernel_id_map_range(root, STACK_START, STACK_END, 	SV39_FLAGS_READ | SV39_FLAGS_WRITE);
	kernel_id_map_range(root, HEAP_START, HEAP_END, 	SV39_FLAGS_READ | SV39_FLAGS_WRITE);
	kernel_id_map_range(root, UART_NS16550A_BASE, UART_NS16550A_BASE + BASE_PAGE_SIZE, SV39_FLAGS_READ | SV39_FLAGS_WRITE);
	fmtprint("[kinit] Kernel paging initialized.\n");

	// Returns the value to write to the SATP register 
	return  SATP_MODE_SV39_FLAG | SATP_PPN_MASK(root);
}

void kmain(void)
{
	fmtprint("[kmain] Paging enabled. Kernel is now running with paging.\n");

	// Main loop of the kernel
	while (1) {
		// Here you would typically handle interrupts, system calls, etc.
	}
}