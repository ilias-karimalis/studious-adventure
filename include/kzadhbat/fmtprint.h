#pragma once

#include <kzadhbat/types/numeric_types.h>

/// Defines the type of a putchar function
typedef void (*putchar_func_t)(char);

void strlib_initialize(putchar_func_t putchar);
void print(const char *fmt, ...);
void println(const char *fmt, ...);
