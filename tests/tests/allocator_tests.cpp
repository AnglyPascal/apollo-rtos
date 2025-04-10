#include "core/memory.h"
#include "core/sched.h"
#include "core/test.h"
#include "utility/allocator.h"
#include "utility/debug.h"

namespace TEST_SUITE(allocator)
{

TEST(allocator_freelist_realloc)
{
  allocator<alloc_heap, 4> pool;

  byte_t *a, *b, *c;

  a = pool.alloc(4);
  pool.dealloc(a);

  b = pool.alloc(4);
  c = pool.alloc(4);

  pool.dealloc(b);
  pool.dealloc(c);

  return a == b && b != c;
}

TEST(allocator_dealloc_reversed_order)
{
  allocator<alloc_heap, 4> pool;

  byte_t *a, *b, *c;

  a = pool.alloc(4);
  b = pool.alloc(16);

  pool.dealloc(a);
  pool.dealloc(b);

  c = pool.alloc(4);
  pool.dealloc(c);

  return a != c && b == c;
}

TEST(allocator_freelist_iter)
{
  allocator<alloc_heap, 4> pool;

  byte_t *a, *b, *c, *d, *e;

  a = pool.alloc(4);
  b = pool.alloc(8);
  c = pool.alloc(16);

  pool.dealloc(b);
  pool.dealloc(c);
  pool.dealloc(a);

  d = pool.alloc(8);
  e = pool.alloc(16);

  pool.dealloc(d);
  pool.dealloc(e);

  return d == c && e != c && e != a && e != b;
}

TEST(allocator_rounding)
{
  bool t1, t2;
  byte_t *a, *b, *c, *d, *e;

  {
    allocator<alloc_heap, 8> pool;

    a = pool.alloc(4);
    b = pool.alloc(8);
    c = pool.alloc(16);

    pool.dealloc(b);
    pool.dealloc(c);
    pool.dealloc(a);

    d = pool.alloc(8);
    e = pool.alloc(16);

    pool.dealloc(d);
    pool.dealloc(e);

    t1 = d == a && e == c;
  }

  {
    allocator<alloc_heap, 16> pool;

    a = pool.alloc(4);
    b = pool.alloc(8);
    c = pool.alloc(16);

    pool.dealloc(c);
    pool.dealloc(a);
    pool.dealloc(b);

    d = pool.alloc(8);
    e = pool.alloc(16);

    pool.dealloc(d);
    pool.dealloc(e);

    t2 = d == b && e == a;
  }

  return t1 && t2;
}

} // namespace TEST_SUITE(allocator)

namespace TEST_SUITE(malloc)
{
SYS_TEST(malloc_freelist_realloc)
{
  void *a, *b, *c;

  a = heap::malloc(4);
  heap::free(a);

  b = heap::malloc(4);
  c = heap::malloc(4);

  heap::free(b);
  heap::free(c);

  return a == b && b != c;
}

SYS_TEST(malloc_dealloc_reversed_order)
{
  void *a, *b, *c;

  a = heap::malloc(4);
  b = heap::malloc(16);

  heap::free(a);
  heap::free(b);

  c = heap::malloc(4);
  heap::free(c);

  return a != c && b == c;
}

SYS_TEST(malloc_freelist_iter)
{
  void *a, *b, *c, *d, *e;

  a = heap::malloc(4);
  b = heap::malloc(8);
  c = heap::malloc(16);

  heap::free(b);
  heap::free(c);
  heap::free(a);

  d = heap::malloc(8);
  e = heap::malloc(16);

  heap::free(d);
  heap::free(e);

  return d == c && e != c && e != a && e != b;
}
} // namespace TEST_SUITE(malloc)

namespace TEST_SUITE(malloc_cleanup)
{
void *a, *b, *c;

PROC(proc0, MID1, 8, param)
{
  a = heap::malloc(4);
  b = heap::malloc(8);
  c = heap::malloc(16);
}

SYS_TEST(malloc_cleanup)
{
  kprintf("malloc cleanup\r\n\r\n");

  auto p0 = REG_PROC(proc0, nullptr);
  sched::wait(p0);

  auto c0 = heap::malloc(16);
  auto b0 = heap::malloc(8);
  auto a0 = heap::malloc(4);

  kprintf("%x, %x, %x, %x, %x, %x\r\n", a, a0, b, b0, c, c0);
  halt();
  return a0 == a && b0 == b && c0 == c;
}
} // namespace TEST_SUITE(malloc_cleanup)
