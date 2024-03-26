#ifndef HELPERS_H
#define HELPERS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* Shorter fixed-width primitive type names. */
#define i8  int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t
#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

/* Math epsilon for comparing floating-point values to 0. */
#define MATH_EPS 0.000001

/* User-level logging */
#define INFO(fmt, ...) do { \
	fprintf(stderr, "ppmtool: " fmt, __VA_ARGS__); \
	fputc('\n', stderr); \
} while (0)

/* Library-level logging */
#define WARN_L(fmt, ...) do { \
	fprintf(stderr, __FILE__":%d:WARN: " fmt, __LINE__, __VA_ARGS__); \
	fputc('\n', stderr); \
} while (0)
#define ERROR_L(fmt, ...) do { \
	fprintf(stderr, __FILE__":%d:ERROR: " fmt, __LINE__, __VA_ARGS__); \
	fputc('\n', stderr); \
} while (0)

#define MAX(A, B) ((A) >= (B) ? (A) : (B))
#define MIN(A, B) ((A) <= (B) ? (A) : (B))
#define CLAMP(X, A, B) MIN(MAX(A, X), B)

/* Mallocs, exits with message in case of failure. */
void *malloc_or_die(size_t nbytes);

/* Callocs, exits with message in case of failure. */
void *calloc_or_die(size_t nbytes);

/* Checks if the host system is little-endian. */
bool host_is_little_endian(void);

/* Converts an u16 to the other endianess. */
u16 u16_to_other_endian(u16 num);

/* Computes variance of a series of numbers. */
double var(const double *nums, size_t len);

/* Partitions numbers into 2 sets, x <= threshold and x > threshold.
 *
 * - `nums`: the numbers to partition (input),
 * - `nums_len`: the length of `nums` (input),
 * - `t`: threshold (input),
 * - `le`: buffer for numbers less-than-or-equal to `t` (output),
 * - `le_len`: the length of `le` (output)
 * - `gt`: buffer for numbers greater-than `t` (output),
 * - `gt_len`: the length of `gt` (output)
 *
 *  `le` and `gt` buffers must be preallocated and not smaller than `nums`. */
void partition(const double *nums, size_t nums_len, double t, double *le, size_t *le_len, double *gt, size_t *gt_len);

#endif /* HELPERS_H */
