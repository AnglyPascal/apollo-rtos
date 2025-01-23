#include "memory.h"
#include "debug.h"

__extern_C__
uint8_t __stack_limit[],
    __end[];

namespace
{
volatile uint8_t *membot = __end;
volatile uint8_t *memtop = __stack_limit;

constexpr uint32_t BLANK_WORD = 0xdeadbeef;
} // namespace

byte_t *alloc_heap(size_t sz)
{
  membot = (uint8_t *)roundup((size_t)membot, 8);

  if (sz > (size_t)(memtop - membot)) {
    return nullptr;
  }

  auto ptr = membot;
  membot += sz;

  // pollute the stack
  for (uint32_t *p = (uint32_t *)ptr; p < (uint32_t *)membot; p++) {
    *p = BLANK_WORD;
  }

  debug<TRACE>("alloc heap: %x\n", ptr);
  return (byte_t *)ptr;
}

byte_t *alloc_stack(size_t sz)
{
  if (sz > (size_t)(memtop - membot))
    return nullptr;

  memtop -= sz;

  for (uint32_t *p = (uint32_t *)memtop; p < (uint32_t *)(memtop + sz); p++) {
    *p = BLANK_WORD;
  }

  debug<TRACE>("alloc stack: %x\n", memtop);
  return (byte_t *)memtop;
}

void *_memcpy(void *dest, const void *src, uint32_t n)
{
  auto *p = static_cast<uint8_t *>(dest);
  auto *q = static_cast<const uint8_t *>(src);
  while (n-- > 0)
    *p++ = *q++;
  return dest;
}

void *_memmove(void *dest, const void *src, uint32_t n)
{
  auto *p = static_cast<uint8_t *>(dest);
  auto *q = static_cast<const uint8_t *>(src);
  if (dest <= src)
    while (n-- > 0)
      *p++ = *q++;
  else {
    p += n;
    q += n;
    while (n-- > 0)
      *--p = *--q;
  }
  return dest;
}

void *_memset(void *dest, uint32_t x, uint32_t n)
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
