#pragma once

#include <grouperlib/numeric_types.h>

typedef void (*putchar_func_t)(char);

void strlib_initialize(putchar_func_t putchar);

void print(const char *str);
void println(const char *str);
void fmtprint(const char *fmt, ...);
void flush(void);
