#include "allocator.h"
#include "memory.h"
#include "sched.h"
#include "serial.h"

namespace sched
{
chunk_t *curr_proc_used_hd();
}

namespace heap
{

namespace
{
allocator<alloc_heap, 8> pool;
}

void *malloc(size_t sz)
{
  auto ptr = pool.alloc(sz);
  auto chunk = (chunk_t *)(ptr - sizeof(chunk_t));

  auto used_hd = sched::curr_proc_used_hd();

  chunk->next = used_hd->next;
  chunk->prev = used_hd;
  chunk->sz = sz;

  if (used_hd->next != nullptr)
    used_hd->next->prev = chunk;
  used_hd->next = chunk;

  return ptr;
}

void free(void *ptr)
{
  if (ptr == nullptr)
    return;

  auto chunk = (chunk_t *)((byte_t *)ptr - sizeof(chunk_t));
  chunk->prev->next = chunk->next;
  if (chunk->next != nullptr)
    chunk->next->prev = chunk->prev;

  pool.dealloc((byte_t *)ptr);
}

void cleanup()
{
  debug<TRACE>("heap cleanup for current proc\r\n");
  auto hd = sched::curr_proc_used_hd();
  while (hd->next != nullptr) {
    auto ptr = (byte_t *)hd->next + sizeof(chunk_t);
    hd = hd->next;
    pool.dealloc(ptr);
  }
}

void trace()
{
  printf("heap allocator: \r\n");
  pool.trace();
}

} // namespace heap

void *operator new(size_t sz)
{
  return heap::malloc(sz);
}

void operator delete(void *ptr)
{
  return heap::free(ptr);
}
