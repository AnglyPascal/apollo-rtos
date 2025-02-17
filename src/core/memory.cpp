#include "core/memory.h"
#include "utility/allocator.h"
#include "utility/debug.h"

__extern_C__
uint8_t __stack_limit[],
    __end[];

namespace
{
volatile uint8_t *membot = __end;
volatile uint8_t *memtop = __stack_limit;

constexpr uint32_t BLANK_WORD = 0xdeadbeef;
} // namespace

byte_t *alloc_heap(size_t nbytes)
{
  if (nbytes > (size_t)(memtop - membot))
    return nullptr;

  auto ptr = membot;
  membot += nbytes;

  debug<TRACE>("alloc heap: %x\r\n", ptr);
  return (byte_t *)ptr;
}

byte_t *alloc_stack(size_t nbytes)
{
  if (nbytes > (size_t)(memtop - membot))
    return nullptr;

  memtop -= nbytes;

  for (auto p = (uint32_t *)memtop; p < (uint32_t *)(memtop + nbytes); p++) {
    *p = BLANK_WORD;
  }

  debug<TRACE>("alloc stack: %x\n", memtop);
  return (byte_t *)memtop;
}

void *_memcpy(void *dest, const void *src, uint32_t n)
{
  auto p = (uint32_t *)dest;
  auto q = (const uint32_t *)src;
  while (n >= sizeof(uint32_t)) {
    *p++ = *q++;
    n -= sizeof(uint32_t);
  }

  auto pb = (uint8_t *)p;
  auto qb = (uint8_t *)q;
  while (n-- > 0) {
    *pb++ = *qb++;
  }

  return dest;
}

void *_memmove(void *dest, const void *src, uint32_t n)
{
  auto *p = (uint8_t *)dest;
  auto *q = (const uint8_t *)src;

  if (dest <= src) {
    while (n-- > 0)
      *p++ = *q++;
  } else {
    p += n;
    q += n;
    while (n-- > 0)
      *--p = *--q;
  }
  return dest;
}

void *_memset(void *dest, uint8_t x, uint32_t n)
{
  auto *p = static_cast<uint8_t *>(dest);
  while (n-- > 0)
    *p++ = x;
  return dest;
}

int _memcmp(const void *pp, const void *qq, int n)
{
  auto *p = static_cast<const uint8_t *>(pp);
  auto *q = static_cast<const uint8_t *>(qq);
  while (n-- > 0) {
    if (*p++ != *q++)
      return (p[-1] < q[-1] ? -1 : 1);
  }
  return 0;
}

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
allocator<alloc_heap, 8> kpool;
}

void *kmalloc(size_t sz) { return kpool.alloc(sz); }

void kfree(void *ptr)
{
  if (ptr == nullptr)
    return;
  kpool.dealloc((byte_t *)ptr);
}
} // namespace kmem

