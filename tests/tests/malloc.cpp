#include "core/memory.h"
#include "core/sched.h"
#include "core/test.h"
#include "utility/debug.h"

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
  auto p0 = REG_PROC(proc0, nullptr);
  sched::wait(p0);

  auto c0 = heap::malloc(16);
  auto b0 = heap::malloc(8);
  auto a0 = heap::malloc(4);

  return a0 == a && b0 == b && c0 == c;
}
} // namespace TEST_SUITE(malloc_cleanup)
