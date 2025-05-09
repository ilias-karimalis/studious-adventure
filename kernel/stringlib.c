/// Buffered stream printing implementation

// Kernel includes
#include <kernel/stringlib.h>
// Kernel libc includes
#include <minilibc/string.h>


struct PrintStream {
	putchar_func_t print;
	usize buffer_len;
	usize i;
	char* buffer;
} ps;

#define StaticMemoryBufferSize 4096
void strlib_initialize(putchar_func_t putchar)
{
	static char initial_buffer[StaticMemoryBufferSize];
	ps = (struct PrintStream) {
		.print = putchar,
		.buffer_len = StaticMemoryBufferSize,
		.i = 0,
		.buffer = initial_buffer
	};
}

int strlib_print_stream_buffer() 
{
	if (ps.buffer == NULL) return 1;
	size_t str_len = strlen(ps.buffer);
	for (usize i = 0; i < str_len; i++) {
		ps.print(ps.buffer[i]);
	}
	memset(ps.buffer, 0, ps.buffer_len);
	ps.i = 0;
	return 0;
}

int strlib_push_char_to_stream(char c) 
{
	ps.buffer[ps.i++] = c;
	if (ps.i == ps.buffer_len - 2 || c == '\n') {
		ps.buffer[ps.i] = '\0';
		return strlib_print_stream_buffer();
	}
	return 0;
}

int strlib_push_str_to_stream(const char* str) 
{
	int err;
	if (str == NULL) return 1;
	while (*str != '\0') {
		err = strlib_push_char_to_stream(*str++);
		if (err != 0) return err;
	}
	return 0;
}

#define MAX_INT_BUF_SIZE 67
int strlib_push_int_to_buf(usize val, usize base) {
	static char buf[MAX_INT_BUF_SIZE] = { 0 };
	usize i = MAX_INT_BUF_SIZE - 2;
	while (val && i) {
		buf[i] = "0123456789abcdefghijklmnopqrstuvwxyz"[val % base];
		i--;
		val /= base;
	}

	switch (base) {
		case 16:
			buf[i--] = 'x';
			buf[i--] = '0';
			break;
		case 8:
			buf[i--] = 'o';
			buf[i--] = '0';
			break;
		case 2:
			buf[i--] = 'b';
			buf[i--] = '0';
			break;
		default:
			break;
	}
	return strlib_push_str_to_stream(&buf[i + 1]);
}

// Public facing printing functions

void print(const char* str)
{
	strlib_push_str_to_stream(str);
}

void flush()
{
	strlib_print_stream_buffer();
}

usize base_buffer[256] = {
	['d'] = 10,
	['x'] = 16,
	['o'] = 8,
	['b'] = 2
};

void fmtprint(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	bool fmt_spec = false;
	char cur;
	while ((cur = *fmt++) != '\0') {
		switch (cur) {
			case '%':
				if (fmt_spec) 
					strlib_push_char_to_stream(cur);
				fmt_spec = !fmt_spec;
				break;

			case 's':
				if (fmt_spec) {
					const char* str = va_arg(args, const char*);
					strlib_push_str_to_stream(str);
					fmt_spec = false;
					break;
				}
				strlib_push_char_to_stream(cur);
				break;
			
			case 'c':
				if (fmt_spec) {
					char c = (char) va_arg(args, int);
					strlib_push_char_to_stream(c);
					fmt_spec = false;
					break;
				}
				strlib_push_char_to_stream(cur);
				break;

			case 'd':
			case 'x':
			case 'o':
			case 'b':
				if (fmt_spec) {
					usize v = va_arg(args, usize);
					strlib_push_int_to_buf(v, base_buffer[(usize) cur]);
					fmt_spec = false;
					break;
				}
				strlib_push_char_to_stream(cur);
				break;

			// We flush the buffer on a new line.
			case '\n':
				strlib_push_char_to_stream(cur);
				strlib_print_stream_buffer();
				break;

			default: 
				strlib_push_char_to_stream(cur);
				break;
		}
	}
}


