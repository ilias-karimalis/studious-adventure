#include <octiron/uart_ns16550a.h>
#include <octiron/pmm.h>
#include <octiron/paging.h>
#include <octiron/devices/device_tree/device_tree.h>

#include <kzadhbat/arch/riscv.h>
#include <kzadhbat/fmtprint.h>
#include <kzadhbat/types/error.h>
#include <kzadhbat/bitmacros.h>
#include <kzadhbat/assert.h>

__attribute__((aligned(4))) void kmain(void);
extern void asm_trap_vector(void);

#define EARLY_HEAP_SIZE (128 * BASE_PAGE_SIZE)
__attribute__((aligned(BASE_PAGE_SIZE))) u8 early_heap[EARLY_HEAP_SIZE] = { 0 };

/// The base physical address for the device tree blob structure.
/// The attribute is necessary here so that the variable doesn't end up in the
/// BSS section which gets zeroed out after this is initialized.
paddr_t dtb_base_addr = 0;


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

void kinit(void)
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
	print("\tBooting Octiron                                        \n");
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
	print("\t* Device Tree Blob Start:   %x\n", dtb_base_addr);
	print("[kinit] Device Tree Blob Start: %x\n", dtb_base_addr);

	// Initialize the physical memory manager
	err = pmm_initialize();
	if (err_is_fail(err)) {
		PANIC_LOOP("[kinit] Failed to initialize pmm: %s\n", err_str(err));
	}
	print("[kinit] Empty pmm initialized.\n");
	err = pmm_add_region(early_heap , EARLY_HEAP_SIZE);
	if (err_is_fail(err)) {
		PANIC_LOOP("[kinit] Failed to add initial pmm region: %s\n", err_str(err));
	}
	print("[kinit] pmm initialized with the early heap memory (%x bytes).\n", pmm_total_mem());

	// Initialize kernel paging
	sv39_pageTable *root = sv39_kernel_page_table();
	// Setting up kernel identity mappings
	kernel_id_map_range(root, TEXT_START, TEXT_END, SV39_FLAGS_READ | SV39_FLAGS_EXECUTE);
	kernel_id_map_range(root, RODATA_START, RODATA_END, SV39_FLAGS_READ);
	kernel_id_map_range(root, DATA_START, DATA_END, SV39_FLAGS_READ | SV39_FLAGS_WRITE);
	kernel_id_map_range(root, BSS_START, BSS_END, SV39_FLAGS_READ | SV39_FLAGS_WRITE);
	kernel_id_map_range(root, STACK_START, STACK_END, SV39_FLAGS_READ | SV39_FLAGS_WRITE);
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
	for (vaddr_t va = UART_NS16550A_BASE; va < UART_NS16550A_BASE + BASE_PAGE_SIZE; va += BASE_PAGE_SIZE) {
		OPT(paddr_t) pa = sv39_virt_to_phys(root, va);
		ASSERT(OPT_EQ(pa, va), "UART: Identity mapping failed, mapping valid: %d, va: %lx, pa: %lx\n", pa.some,
		       va, pa.val);
	}

	// Set the sstatus register such that we can use the supervisor mode
	csrw_sstatus((1 << 8) | (1 << 5));
	// Set the spec register to point to the kernel entry point
	csrw_sepc((u64) kmain);
	// Set the mideleg register sych that software, timer and external interrupts are delegated to the supervisor mode
	csrw_mideleg((1 << 1) | (1 << 5) | (1 << 9));
	// Set the sie register to match the value of mideleg
	csrw_sie((1 << 1) | (1 << 5) | (1 << 9));
	// Set the stvec register to point to the kerne's trap handler
	csrw_stvec((u64) asm_trap_vector);
	// Set the satp value to the root of the kernel page table with the SV39 mode enabled
	csrw_satp(SATP_MODE_SV39_FLAG | SATP_PPN_MASK(root));

	// Setup pmp to map the whole physical address space
	csrw_pmpaddr0(0);
	csrw_pmpcfg0(0xF);

	// Fence to ensure that the CPU has taken our SATP register
	sfence_vma();

	// Return to the kernel entry point (kmain)
	sret();
}

/// Note: The value of sepc (address of `kmain`) needs to have it's lower to bits be zeroed
__attribute__((aligned(4))) void kmain(void)
{
	errval_t err = ERR_OK;
	print("[kmain] Paging enabled. Kernel is now running with paging.\n");

	// We parse the DTB block at this point because we need to figure out the special memory regions which should
	// not be included in the Kernel Heap (including the dtb mapping itself).
	err = dt_initialize(dtb_base_addr);
	if (err_is_fail(err)) {
		PANIC_LOOP("[kmain] Failed to parse DTB: %s\n", err_str(err));
	}

	// Main loop of the kernel
	print("[kmain] Kernel loop reached.\n");
	while (1) {
		// Here you would typically handle interrupts, system calls, etc.
	}
}
