#pragma once

#include "types.h"
#include <cstdint>

struct chunk_t {
  chunk_t *next = nullptr;
  chunk_t *prev = nullptr;

  size_t sz = 0;
};

constexpr size_t roundup(size_t sz, size_t align)
{
  return (sz + align - 1) & ~(align - 1);
}

/* allocate space at the bottom of the heap */
byte_t *alloc_heap(size_t sz);

/* allocate space for a stack using sbrk */
byte_t *alloc_stack(size_t sz);

/* copy n bytes from src to dest (non-overlapping) */
void *_memcpy(void *dest, const void *src, uint32_t n);

/* copy n bytes from src to dest, allowing overlaps */
void *_memmove(void *dest, const void *src, uint32_t n);

/* set n bytes of dest to byte x */
void *_memset(void *dest, uint8_t x, uint32_t n);

/* compare n bytes */
int _memcmp(const void *pp, const void *qq, int n);

namespace heap
{
void *malloc(size_t sz) __attribute__((malloc));
void free(void *);
void trace();
void cleanup();
} // namespace heap

void *operator new(size_t sz);
void operator delete(void *);
