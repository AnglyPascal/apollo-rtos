#pragma once

#include "types.h"
#include <cstdint>

constexpr size_t roundup(size_t sz, size_t align)
{
  return (sz + align - 1) & ~(align - 1);
}

/* allocate space at the bottom of the heap */
uint8_t *alloc_heap(size_t sz);

/* allocate space for a stack using sbrk */
uint8_t *alloc_stack(size_t sz);

/* copy n bytes from src to dest (non-overlapping) */
void *_memcpy(void *dest, const void *src, uint32_t n);

/* copy n bytes from src to dest, allowing overlaps */
void *_memmove(void *dest, const void *src, uint32_t n);

/* set n bytes of dest to byte x */
void *_memset(void *dest, uint32_t x, uint32_t n);

/* compare n bytes */
int _memcmp(const void *pp, const void *qq, int n);

namespace heap
{
void *malloc(size_t sz);
void free(void *);
void trace();
} // namespace heap

void *operator new(size_t sz);
void operator delete(void *);
