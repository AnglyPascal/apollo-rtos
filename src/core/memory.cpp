#include "core/memory.h"
#include "utility/allocator.h"
#include "utility/debug.h"

__extern_C__ uint8_t __stack_limit[], __end[];

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

  for (auto p = (uint32_t *)memtop; p < (uint32_t *)(memtop + nbytes); p++)
    *p = BLANK_WORD;

  debug<TRACE>("alloc stack: %x\n", memtop);
  return (byte_t *)memtop;
}

extern "C" void *memcpy(void *dest, const void *src, uint32_t n)
{
  auto *d = (uint8_t *)dest;
  auto *s = (const uint8_t *)src;

  // copy word-aligned when possible
  if ((((size_t)d | (size_t)s) & (sizeof(uint32_t) - 1)) == 0) {
    auto *dw = (uint32_t *)d;
    auto *sw = (const uint32_t *)(s);
    while (n >= sizeof(uint32_t)) {
      *dw++ = *sw++;
      n -= sizeof(uint32_t);
    }
    d = (uint8_t *)dw;
    s = (const uint8_t *)sw;
  }

  while (n--) {
    *d++ = *s++;
  }

  return dest;
}

extern "C" void *memset(void *dest, uint8_t x, uint32_t n)
{
  auto *p = static_cast<uint8_t *>(dest);
  while (n-- > 0)
    *p++ = x;
  return dest;
}

extern "C" int memcmp(const void *pp, const void *qq, int n)
{
  auto *p = static_cast<const uint8_t *>(pp);
  auto *q = static_cast<const uint8_t *>(qq);
  while (n-- > 0) {
    if (*p++ != *q++)
      return (p[-1] < q[-1] ? -1 : 1);
  }
  return 0;
}

extern "C" void strcpy(char *dest, const char *src)
{
  while (*src != '\0')
    *dest++ = *src++;
  *dest = '\0';
}

extern "C" int strcmp(const char *lhs, const char *rhs)
{
  if (lhs == nullptr && rhs == nullptr)
    return 0;

  if (lhs == nullptr)
    return -1;

  if (rhs == nullptr)
    return 1;

  while (*rhs != '\0' && *lhs != '\0' && *lhs == *rhs) {
    lhs++;
    rhs++;
  }

  if (*lhs < *rhs)
    return -1;
  if (*lhs > *rhs)
    return 1;
  return 0;
}

namespace curr_proc
{
chunk_list_t &used_list();
} // namespace curr_proc

namespace heap
{
allocator<alloc_heap, 4> pool;

inline chunk_t *ptr_to_chunk(void *ptr)
{
  return (chunk_t *)((byte_t *)ptr - sizeof(chunk_t));
}

inline byte_t *chunk_to_ptr(chunk_t *chunk)
{
  return (byte_t *)chunk + sizeof(chunk_t);
}

void *malloc(size_t sz)
{
  auto ptr = pool.alloc(sz);
  if (ptr == nullptr)
    return ptr;

  auto chunk = ptr_to_chunk(ptr);
  auto &used_list = curr_proc::used_list();
  used_list.push_back(chunk);

  return ptr;
}

void free(void *ptr)
{
  if (ptr == nullptr)
    return;

  auto chunk = ptr_to_chunk(ptr);
  chunk->detach();

  pool.dealloc((byte_t *)ptr);
}

void cleanup()
{
  auto &used_list = curr_proc::used_list();

  debug<TRACE>("heap cleanup for current proc\r\n");
  while (used_list.begin() != used_list.end()) {
    auto &chunk = *used_list.begin();
    chunk.detach();

    auto ptr = chunk_to_ptr(&chunk);
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
allocator<alloc_heap, 4> kpool;
void *kmalloc(size_t sz) { return kpool.alloc(sz); }
void kfree(void *ptr) { kpool.dealloc((byte_t *)ptr); }
} // namespace kmem

namespace sched
{
void init_lists();
}

namespace
{
INIT_FUNC()
{
  heap::pool.init_list();
  kmem::kpool.init_list();
}
} // namespace

