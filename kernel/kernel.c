#include <kernel/uart_ns16550a.h>
#include <kernel/stringlib.h>
 
void kmain(void) {
	//print("Hello world!\r\n");
	uart_ns16550a_initialize(UART_NS16550A_BASE);

	// Initialize the string library with the UART functions
	strlib_initialize(uart_ns16550a_putchar);

	// Print the greeting visual
	print("                    _,,......_				\n");	
	print("                 ,-'          `'--.			\n");	
	print("              ,-'  _              '-.			\n");	
	print("     (`.    ,'   ,  `-.              `.			\n");	
	print("      \\ \\  -    / )    \\               \\			\n");	
	print("       `\\`-^^^, )/      |     /         :		\n");	
	print("         )^ ^ ^V/            /          '.		\n");	
	print("         |      )            |           `.		\n");	
	print("         9   9 /,--,\\    |._:`         .._`.		\n");	
	print("         |    /   /  `.  \\    `.      (   `.`.		\n");	
	print("         |   / \\  \\    \\  \\     `--\\   )    `.`.__	\n");	
	print("-hrr-   .;;./  '   )   '   )       ///'       `-\"	\n");	
	print("        `--'   7//\\    ///\\				\n");	
	print("								\n");	
	print("=========================================================\n");
	print("\tBooting Ilias' toy OS					\n");
	print("=========================================================\n");

	fmtprint("[kmain] uart NS16550A initialized @ %x\n", UART_NS16550A_BASE);

	print("Echo: \n");
	while(1) {
		// Read input from the UART
		char c = uart_ns16550a_getchar();
		// Echo the character back
		uart_ns16550a_putchar(c);
	}
	return;
}

