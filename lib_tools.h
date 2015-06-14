#pragma once

#include <stdio.h>

typedef char bool;

#define true 1
#define false 0

#define unused __attribute__((unused))

#define error(message)	fprintf(stderr, "[%s:%d] %s\n", __FILE__, __LINE__, (message))

#define	inf	__builtin_huge_val()
#define finf	__builtin_huge_valf()
#define	to_int(val)	__builtin_nan(val)
#define	to_float(val)	__builtin_nanf(val)
#define epsilon	0.0000001

#define nnull	__attribute__((nonnull))
#define nonnull(id, ...)	__attribute__((nonnull (id, ##__VA_ARGS__)))
