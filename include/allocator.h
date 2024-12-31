#pragma once

#include "memory.h"
#include "types.h"

class allocator
{
  struct chunk_t {
    chunk_t *next = nullptr;
    size_t sz = 0;
  };

  using allocator_t = uint8_t *(*)(size_t);

  allocator_t alloc_func;
  chunk_t head;
  size_t alignment;
  size_t header_sz;

public:
  allocator() = delete;
  constexpr allocator(allocator_t alloc_func, size_t alignment)
      : alloc_func{alloc_func}, head{0}, alignment{alignment},
        header_sz{roundup(sizeof(chunk_t), alignment)}
  {
  }

  uint8_t *alloc(size_t sz);
  void dealloc(uint8_t *ptr);

  void trace();
};
