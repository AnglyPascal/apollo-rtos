#include "utility/allocator.h"
#include "core/memory.h"
#include "core/sched.h"
#include "drivers/serial.h"

namespace curr_proc
{
chunk_t *used_hd();
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

  auto used_hd = curr_proc::used_hd();
  used_hd->insert_next(chunk);

  return ptr;
}

void free(void *ptr)
{
  if (ptr == nullptr)
    return;

  auto chunk = (chunk_t *)((byte_t *)ptr - sizeof(chunk_t));
  chunk->detach();

  pool.dealloc((byte_t *)ptr);
}

void cleanup(chunk_t *hd)
{
  debug<TRACE>("heap cleanup for current proc\r\n");
  while (hd->next != nullptr) {
    assert(hd->next != hd);
    hd->next->detach();

    auto ptr = (byte_t *)hd->next + sizeof(chunk_t);
    pool.dealloc(ptr);
  }
}

void trace()
{
  debug<TRACE>("  heap: \r\n");
  pool.trace();
}

} // namespace heap

void *operator new(size_t sz) { return heap::malloc(sz); }

void operator delete(void *ptr) { return heap::free(ptr); }

namespace kmem
{
namespace
{
allocator<alloc_heap, 4> pool;
}

void *kmalloc(size_t sz) { return pool.alloc(sz); }

void kfree(void *ptr)
{
  if (ptr == nullptr)
    return;
  pool.dealloc((byte_t *)ptr);
}
} // namespace kmem

