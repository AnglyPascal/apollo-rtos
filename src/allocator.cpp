#include "allocator.h"
#include "memory.h"
#include "serial.h"
#include "types.h"

uint8_t *allocator::alloc(size_t sz)
{
  sz = roundup(sz, alignment);
  auto chnk_sz = header_sz + sz;

  auto ptr = &head;

  while (ptr->next != nullptr) {
    auto chunk = ptr->next;
    ptr->next = chunk->next;

    if (chunk->sz >= sz) {
      return (uint8_t *)chunk + header_sz;
    }
  }

  auto chunk = (chunk_t *)alloc_func(chnk_sz);
  chunk->sz = sz;
  return (uint8_t *)chunk + header_sz;
}

void allocator::dealloc(uint8_t *ptr)
{
  auto chunk = (chunk_t *)(ptr - header_sz);
  chunk->next = head.next;
  head.next = chunk;
}

void allocator::trace()
{
  printf("alloc trace: \n");
}
