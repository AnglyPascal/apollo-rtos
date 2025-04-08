#include "core/test.h"
#include "drivers/display.h"
#include "utility/allocator.h"
#include "utility/debug.h"

namespace
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

TEST(allocator_dealloc_reversed)
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

TEST(allocator_freelist_iter_rounding)
{
  allocator<alloc_heap, 8> pool;

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

  return d == a && e == c;
}

} // namespace

