#pragma once

#include "core/types.h"
#include "utility/debug.h"
#include <cstdint>

struct chunk_t {
  chunk_t *next = nullptr;
  chunk_t *prev = nullptr;

  size_t sz = 0;

  void insert_next(chunk_t *chunk)
  {
    chunk->next = next;
    chunk->prev = this;

    if (next != nullptr)
      next->prev = chunk;
    next = chunk;
  }

  void detach()
  {
    if (prev != nullptr)
      prev->next = next;
    if (next != nullptr)
      next->prev = prev;

    prev = nullptr;
    next = nullptr;
  }
};

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
void cleanup(chunk_t *);
} // namespace heap

void *operator new(size_t sz);
void operator delete(void *);

namespace kmem
{
void *kmalloc(size_t sz) __attribute__((malloc));
void kfree(void *);
} // namespace kmem
