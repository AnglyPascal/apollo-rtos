#include "allocator.h"
#include "serial.h"
#include "types.h"

uint8_t *allocator::alloc(size_t sz)
{
  auto chnk_sz = chunk_header_sz + sz;
  auto ptr = &head;

  while (ptr->next != nullptr) {
    auto chunk = ptr->next;
    ptr->next = ptr->next->next;

    if (chunk->sz >= sz) {
      return chunk->block;
    }
  }

  auto chunk = (chunk_t *)sbrk(chnk_sz);
  chunk->sz = sz;
  return chunk->block;
}

void allocator::dealloc(uint8_t *ptr)
{
  ptr -= chunk_header_sz;
  auto chunk = (chunk_t *)ptr;
  chunk->next = head.next;
  head.next = chunk;
}

void allocator::trace()
{
  serial::printf("alloc trace: \n");
}
