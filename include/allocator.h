#pragma once

#include "debug.h"
#include "irq.h"
#include "memory.h"
#include "serial.h"
#include "types.h"

using allocator_t = byte_t *(*)(size_t);

template <allocator_t alloc_func, size_t alignment>
class allocator
{
  chunk_t free_hd; // singly list

  static constexpr size_t header_sz = sizeof(chunk_t);

public:
  constexpr allocator() {}

  byte_t *alloc(size_t sz)
  {
    intr_guard guard;

    sz = roundup(sz, alignment);
    auto chnk_sz = header_sz + sz;
    chunk_t *chunk = nullptr;

    auto ptr = &free_hd;
    while (ptr->next != nullptr) {
      chunk = ptr->next;
      if (chunk->sz >= sz)
        break;
      ptr = chunk;
    }

    if (chunk == nullptr) {
      chunk = (chunk_t *)alloc_func(chnk_sz);
      chunk->sz = sz;
    } else {
      chunk->detach();
    }

    return (byte_t *)chunk + header_sz;
  }

  void dealloc(byte_t *ptr)
  {
    if (ptr == nullptr)
      return;

    intr_guard guard;
    auto chunk = (chunk_t *)(ptr - header_sz);
    free_hd.insert_next(chunk);
  }

  __noinline__
  void trace()
  {
    debug<TRACE>("  |  free list: \r\n");
    for (auto ptr = &free_hd; ptr->next != nullptr; ptr = ptr->next) {
      debug<TRACE>("  |    %x: %u\r\n", (byte_t *)ptr->next + header_sz,
                   ptr->next->sz);
    }
  }
};
