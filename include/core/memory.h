#pragma once

#include "core/types.h"
#include "utility/debug.h"
#include <cstdint>

byte_t *alloc_heap(size_t sz);
byte_t *alloc_stack(size_t sz);

extern "C" void *memcpy(void *dest, const void *src, uint32_t n);
extern "C" void *memset(void *dest, uint8_t x, uint32_t n);
extern "C" int memcmp(const void *pp, const void *qq, int n);

extern "C" void strcpy(char *dest, const char *src);
extern "C" int strcmp(const char *lhs, const char *rhs);

namespace mem
{
void init();
}

namespace heap
{
void *malloc(size_t sz) __attribute__((malloc));
void free(void *);
void trace();
} // namespace heap

void *operator new(size_t sz);
void operator delete(void *);

namespace kmem
{
void *kmalloc(size_t sz) __attribute__((malloc));
void kfree(void *);

template <typename T, typename... Args>
  requires(!std::is_reference_v<T>)
T *knew(Args &&...args)
{
  auto ptr = kmalloc(sizeof(T));
  return new (ptr) T{std::forward<Args>(args)...};
}
} // namespace kmem
