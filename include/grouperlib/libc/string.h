#pragma once

#include <grouperlib/freestanding.h>

#ifndef NULL
#define NULL 0
#endif


// String Manipulation

// String Examination
size_t strlen(const char *str);

// Character Array Manipulation
void *memset(void *dest, int ch, size_t count);
