#pragma once

#include <kzadhbat/types/numeric_types.h>

#define UART_NS16550A_BASE ((size_t) 0x10000000)

enum UART_NS16550A_Registers {
	/// Receiver Holding Register
	UART_NS16550A_RHR = 0b000,
	/// Transmitter Holding Register
	UART_NS16550A_THR = 0b000,
	/// Interrupt Enable Register
	UART_NS16550A_IER = 0b001,
	/// Interrupt Status Register
	UART_NS16550A_ISR = 0b010,
	/// FIFO Control Register
	UART_NS16550A_FCR = 0b010,
	/// Line Control Register
	UART_NS16550A_LCR = 0b011,
	/// Modem Control Register
	UART_NS16550A_MCR = 0b100,
	/// Line Status Register
	UART_NS16550A_LSR = 0b101,
	/// Modem Status Register
	UART_NS16550A_MSR = 0b110,
	/// Scratch Pad Register
	UART_NS16550A_SPR = 0b111,
	/// Divisor Latch (least significant byte)
	UART_NS16550A_DLL = 0b000,
	/// Divisor Latch (most significant byte)
	UART_NS16550A_DLM = 0b001,
	/// Prescaler Division
	UART_NS16550A_PSD = 0b101,
};

void uart_ns16550a_initialize(size_t base);
void uart_ns16550a_putchar(char c);
char uart_ns16550a_getchar();
