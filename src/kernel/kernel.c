#include <kernel/uart_ns16550a.h>
#include <kernel/pmm.h>

#include <grouperlib/arch/riscv.h>
#include <grouperlib/fmtprint.h>
#include <grouperlib/error.h>

static struct pmm pmm;

void kmain(void)
{
	errval_t err;

	//print("Hello world!\r\n");
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

	fmtprint("[kmain] uart NS16550A initialized @ %x.\n",
		 UART_NS16550A_BASE);

	fmtprint("[kmain] Global Values:\n");
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
	fmtprint("\t* Kernel Stack Start:       %x\n", KERNEL_STACK_START);
	fmtprint("\t* Kernel Stack End:         %x\n", KERNEL_STACK_END);

// Initialize the physical memory manager
#define INITIAL_PMM_SLAB_SZ SLAB_REGION_SIZE(64, sizeof(struct pmmRegion))
	static u8 pmm_initial_region_slab[INITIAL_PMM_SLAB_SZ];
	err = pmm_initialize(&pmm, (u8 *)&pmm_initial_region_slab,
			     INITIAL_PMM_SLAB_SZ);
	err = pmm_add_region(&pmm, (uintptr_t)HEAP_START, (size_t)HEAP_SIZE);
	fmtprint("[kmain] pmm initialized with %x bytes of memory.\n",
		 pmm_total_mem(&pmm));
	static u8 pmm_extra_region[8192] = {0};
	err = pmm_add_region(&pmm, (uintptr_t)pmm_extra_region,
			     sizeof(pmm_extra_region));
	fmtprint("[kmain] pmm initialized with %x bytes of memory.\n",
		 pmm_total_mem(&pmm));

    fmtprint("hi!\n");

	while (1) {
		// Read input from the UART
		char c = uart_ns16550a_getchar();
		// Echo the character back
		uart_ns16550a_putchar(c);
	}
	return;
}
