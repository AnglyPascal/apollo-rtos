#pragma once

#include "debug.h"
#include "irq.h"
#include "memory.h"
#include "serial.h"
#include "types.h"

using allocator_t = byte_t *(*)(size_t);

#define USED_LIST 0

template <allocator_t alloc_func, size_t alignment>
class allocator
{
  chunk_t free_hd; // singly list
                   
#if USED_LIST
  chunk_t used_hd; // doubly list
#endif

  static constexpr size_t header_sz = roundup(sizeof(chunk_t), alignment);

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
      ptr->next = chunk->next;

      if (chunk->sz >= sz)
        break;
    }
    if (chunk == nullptr)
      chunk = (chunk_t *)alloc_func(chnk_sz);

#if USED_LIST
    chunk->prev = &used_hd;
    chunk->next = used_hd.next;

    if (used_hd.next != nullptr)
      used_hd.next->prev = chunk;
    used_hd.next = chunk;
#endif

    chunk->sz = sz;
    return (byte_t *)chunk + header_sz;
  }

  void dealloc(byte_t *ptr)
  {
    intr_guard guard;

    auto chunk = (chunk_t *)(ptr - header_sz);

#if USED_LIST
    chunk->prev->next = chunk->next;
    if (chunk->next != nullptr)
      chunk->next->prev = chunk->prev;

    chunk->prev = nullptr;
#endif

    chunk->next = free_hd.next;
    free_hd.next = chunk;
  }

  __noinline__
  void trace()
  {
    printf("alloc trace: \r\n");
    printf("\tfree list: \r\n");
    for (auto ptr = &free_hd; ptr->next != nullptr; ptr = ptr->next) {
      printf("\t\t%x: %u\r\n", (byte_t *)ptr->next + header_sz, ptr->next->sz);
    }

#if USED_LIST
    printf("\tused list: \r\n");
    for (auto ptr = &used_hd; ptr->next != nullptr; ptr = ptr->next) {
      printf("\t\t%x: %u\r\n", (byte_t *)ptr->next + header_sz, ptr->next->sz);
    }
#endif
  }
};
