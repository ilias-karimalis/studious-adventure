#pragma once

#include <kzadhbat/freestanding.h>

#ifndef NULL
#define NULL 0
#endif

// String Manipulation

// String Examination
size_t strlen(const char *str);
int strcmp(const char *lhs, const char *rhs);
int strncmp(const char *lhs, const char *rhs, size_t num);

// Character Array Manipulation
void *memset(void *dest, int ch, size_t count);
void *memcpy(void *dest, const void *src, size_t count);