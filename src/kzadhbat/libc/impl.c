#include <kzadhbat/libc/string.h>

// String Manipulation

// String Examination

size_t strlen(const char *str)
{
	const char *end = str;
	while (*end != '\0')
		++end;
	return end - str;
}

int strcmp(const char *lhs, const char *rhs)
{
	while (*lhs && (*lhs == *rhs)) {
		lhs++;
		rhs++;
	}
	return *(unsigned char *)lhs - *(unsigned char *)rhs;
}

int strncmp(const char *lhs, const char *rhs, size_t num)
{
	while (num && *lhs && (*lhs == *rhs)) {
		lhs++;
		rhs++;
		num--;
	}
	if (num == 0) {
		return 0; // Strings are equal up to num characters
	}
	return *(unsigned char *)lhs - *(unsigned char *)rhs;
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

void *memcpy(void *dest, const void *src, size_t count)
{
	unsigned char *d = (unsigned char *)dest;
	const unsigned char *s = (const unsigned char *)src;
	for (size_t i = 0; i < count; i++) {
		d[i] = s[i];
	}
	return dest;
}