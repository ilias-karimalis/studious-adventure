/// Formatted string printing

// Kernel includes
#include <grouperlib/fmtprint.h>
// Kernel libc includes
#include <grouperlib/libc/string.h>

putchar_func_t putchar_func = NULL;

void 
strlib_initialize(putchar_func_t putchar)
{
	if (putchar == NULL) {
		return;
	}
	putchar_func = putchar;
}

void 
strlib_print_str(const char* str)
{
	if (str == NULL) return;
	while (*str != '\0') {
		putchar_func(*str++);
	}
}

void 
strlib_print_int(size_t val, size_t base)
{
	#define MAX_INT_BUF_SIZE 67
	static char buf[MAX_INT_BUF_SIZE] = { 0 };
	size_t i = MAX_INT_BUF_SIZE - 2;
	while (val && i) {
		buf[i] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[val % base];
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
	strlib_print_str(&buf[i + 1]);
}

// Public facing printing functions
void 
println(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	print(fmt, args);
	va_end(args);
	putchar_func('\n');
}

size_t base_buffer[256] = { ['d'] = 10, ['x'] = 16, ['o'] = 8, ['b'] = 2 };
void 
print(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	bool fmt_spec = false;
	char cur;
	while ((cur = *fmt++) != '\0') {
		switch (cur) {
		case '%':
			if (fmt_spec)
				putchar_func(cur);
			fmt_spec = !fmt_spec;
			break;

		case 's':
			if (fmt_spec) {
				const char *str = va_arg(args, const char *);
				strlib_print_str(str);
				fmt_spec = false;
				break;
			}
			putchar_func(cur);
			break;

		case 'c':
			if (fmt_spec) {
				char c = (char)va_arg(args, int);
				putchar_func(c);
				fmt_spec = false;
				break;
			}
			putchar_func(cur);
			break;

		case 'd':
		case 'x':
		case 'o':
		case 'b':
			if (fmt_spec) {
				size_t v = va_arg(args, size_t);
				strlib_print_int(v, base_buffer[(size_t)cur]);
				fmt_spec = false;
				break;
			}
			putchar_func(cur);
			break;

		default:
			putchar_func(cur);
			break;
		}
	}
}
