#include <octiron/uart_ns16550a.h>
#include <kzadhbat/types/numeric_types.h>

static volatile u8 *uart_base = 0x0;

void uart_ns16550a_initialize(size_t base)
{
	// Currently we're not really using the full 16550 and are not enabling
	// any of the options. Instead, we're simply polling for get/put.
	uart_base = (volatile u8 *)base;
}

void uart_ns16550a_putchar(char c)
{
	uart_base[UART_NS16550A_THR] = c;
}

char uart_ns16550a_getchar()
{
	while ((uart_base[UART_NS16550A_LSR] & 0x01) == 0) {
	}
	return (char)uart_base[UART_NS16550A_RHR];
}
