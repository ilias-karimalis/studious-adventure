#include <grouperlib/libc/string.h>

// String Manipulation

// String Examination

size_t strlen(const char *str)
{
	const char *end = str;
	while (*end != '\0')
		++end;
	return end - str;
}

// Character Array Manipulation

void *memset(void *dest, int ch, size_t count)
{
	unsigned char *dst = (unsigned char *)dest;
	for (size_t i = 0; i < count; i++) {
		dst[i] = (unsigned char)ch;
	}
	return dest;
}
