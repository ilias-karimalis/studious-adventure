#pragma once

#include <grouperlib/fmtprint.h>

#define ASSERT(expr, msg, ...)                          \
    do {                                                \
        if (!(expr)) {                                  \
            fmtprint(msg __VA_OPT__(,) __VA_ARGS__);    \
            flush();                                    \
            for (;;) { }                                \
        }                                               \
    } while (0)


