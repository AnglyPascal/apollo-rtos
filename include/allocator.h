#pragma once

#include "irq.h"
#include "memory.h"
#include "serial.h"
#include "types.h"

using allocator_t = byte_t *(*)(size_t);

template <allocator_t alloc_func, size_t alignment>
class allocator
{
  struct chunk_t {
    chunk_t *next = nullptr;
    size_t sz = 0;
  };

  chunk_t head;
  static constexpr size_t header_sz = roundup(sizeof(chunk_t), alignment);

public:
  constexpr allocator() {}

  byte_t *alloc(size_t sz)
  {
    intr_guard guard;

    sz = roundup(sz, alignment);
    auto chnk_sz = header_sz + sz;

    auto ptr = &head;
    while (ptr->next != nullptr) {
      auto chunk = ptr->next;
      ptr->next = chunk->next;

      if (chunk->sz >= sz) {
        return (byte_t *)chunk + header_sz;
      }
    }

    auto chunk = (chunk_t *)alloc_func(chnk_sz);
    chunk->sz = sz;
    return (byte_t *)chunk + header_sz;
  }

  void dealloc(byte_t *ptr)
  {
    intr_guard guard;

    auto chunk = (chunk_t *)(ptr - header_sz);
    chunk->next = head.next;
    head.next = chunk;
  }

  __noinline__
  void trace()
  {
    printf("alloc trace: \r\n");
    for (auto ptr = &head; ptr->next != nullptr; ptr = ptr->next) {
      printf("\t%x: %u\r\n", (byte_t *)ptr->next + header_sz, ptr->next->sz);
    }
  }
};
