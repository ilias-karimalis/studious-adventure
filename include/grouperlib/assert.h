#pragma once

#include <grouperlib/fmtprint.h>

#define ASSERT(expr, msg, ...)                          \
    do {                                                \
        if (!(expr)) {                                  \
            print(msg __VA_OPT__(,) __VA_ARGS__);       \
            for (;;) { }                                \
        }                                               \
    } while (0)

#define TODO(msg, ...)                                      \
    do {                                                    \
        print("TODO: " msg __VA_OPT__(,) __VA_ARGS__);      \
        for (;;) { }                                        \
    } while (0)

