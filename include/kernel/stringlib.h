#pragma once

#include <kernel/types.h>

typedef void (*putchar_func_t)(char c);

void strlib_initialize(putchar_func_t putchar);

void print(const char* str);
void fmtprint(const char* fmt, ...);
void flush(void);

