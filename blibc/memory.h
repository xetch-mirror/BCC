#ifndef MEMORY_H
#define MEMORY_H

/* Bold C Library - memory operations
 * Freestanding, no dependency on the host libc.
 */

typedef unsigned long size_t;

/* Copy n bytes from src to dst. Regions must not overlap. */
void *memcpy(void *dst, const void *src, size_t n);

/* Copy n bytes from src to dst. Regions may overlap. */
void *memmove(void *dst, const void *src, size_t n);

/* Fill the first n bytes of dst with byte value c. */
void *memset(void *dst, int c, size_t n);

/* Compare the first n bytes of a and b. Returns <0, 0, >0. */
int memcmp(const void *a, const void *b, size_t n);

/* Find the first occurrence of byte c in the first n bytes of s. */
void *memchr(const void *s, int c, size_t n);

/* Zero the first n bytes of dst. */
void *memzero(void *dst, size_t n);

#endif /* BLIBC_MEMORY_H */
