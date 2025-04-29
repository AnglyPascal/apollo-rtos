#include "core/memory.h"
#include "core/test.h"
#include "utility/allocator.h"
#include "utility/debug.h"

BEGIN_SUITE(allocator)

TEST(basic_allocation_and_reuse)
{
  bool result = true;

  { // Simple allocate/deallocate cycle
    allocator<alloc_heap, 4> pool;
    byte_t *a = pool.alloc(4);
    byte_t *b = pool.alloc(4);
    pool.dealloc(a);
    pool.dealloc(b);

    byte_t *c = pool.alloc(4);
    byte_t *d = pool.alloc(4);
    result &= c == a;
    result &= d == b;
    result &= (c != d);
  }

  { // Mixed allocation/deallocation
    allocator<alloc_heap, 4> pool;
    byte_t *a, *b, *c;

    a = pool.alloc(4);
    pool.dealloc(a);

    b = pool.alloc(4);
    c = pool.alloc(4);
    pool.dealloc(b);
    pool.dealloc(c);

    result &= (a == b && b != c);
  }

  { // Exact size matching
    allocator<alloc_heap, 8> pool;
    byte_t *ptrs[3];
    for (int i = 0; i < 3; i++)
      ptrs[i] = pool.alloc(8);
    for (int i = 0; i < 3; i++)
      pool.dealloc(ptrs[i]);

    for (int i = 0; i < 3; i++)
      result &= pool.alloc(8) == ptrs[i];
  }

  return result;
}

TEST(freelist_management)
{
  bool result = true;

  { // FIFO ordering verification
    allocator<alloc_heap, 4> pool;
    byte_t *a = pool.alloc(4);
    byte_t *b = pool.alloc(4);
    pool.dealloc(a);
    pool.dealloc(b);

    result &= (pool.alloc(4) == a);
    result &= (pool.alloc(4) == b);
  }

  { // Best fit allocation
    allocator<alloc_heap, 8> pool;
    pool.alloc(8); // Allocate and leave in freelist
    byte_t *small = pool.alloc(8);
    byte_t *large = pool.alloc(32);

    pool.dealloc(small);
    pool.dealloc(large);

    // Should pick smallest fitting chunk
    result &= (pool.alloc(8) == small);
    result &= (pool.alloc(32) == large);
  }

  return result;
}

TEST(memory_reuse_scenarios)
{
  bool result = true;

  { // Mixed size allocations
    allocator<alloc_heap, 8> pool;
    byte_t *small = pool.alloc(8);
    byte_t *medium = pool.alloc(16);
    byte_t *large = pool.alloc(32);

    pool.dealloc(medium);
    pool.dealloc(large);
    pool.dealloc(small);

    result &= (pool.alloc(32) == large);
    result &= (pool.alloc(16) == medium);
    result &= (pool.alloc(8) == small);
  }

  { // Fragmentation test
    allocator<alloc_heap, 8> pool;
    byte_t *a = pool.alloc(16);
    pool.alloc(16);
    byte_t *c = pool.alloc(16);

    pool.dealloc(a);
    pool.dealloc(c);

    // Should allocate new chunk instead of splitting
    byte_t *d = pool.alloc(32);
    result &= (d != a && d != c);
  }

  return result;
}

TEST(edge_cases)
{
  bool result = true;

  { // Zero-size allocation
    allocator<alloc_heap, 8> pool;
    byte_t *ptr = pool.alloc(0);
    result &= (ptr == nullptr);
  }

  { // Multiple pool isolation
    allocator<alloc_heap, 8> pool1;
    allocator<alloc_heap, 8> pool2;
    byte_t *a = pool1.alloc(8);
    byte_t *b = pool2.alloc(8);
    result &= (a != b);
  }

  return result;
}

TEST(rounding)
{
  bool result = true;

  { // Rounding behavior
    allocator<alloc_heap, 8> pool;
    byte_t *a, *b, *c, *d, *e;

    a = pool.alloc(4);
    b = pool.alloc(8);
    c = pool.alloc(16);
    pool.dealloc(a);
    pool.dealloc(b);
    pool.dealloc(c);

    d = pool.alloc(8);
    e = pool.alloc(16);
    pool.dealloc(d);
    pool.dealloc(e);

    result &= (d == a && e == c);
  }

  {
    allocator<alloc_heap, 16> pool;
    byte_t *a, *b, *c, *d, *e;

    a = pool.alloc(4);
    b = pool.alloc(8);
    c = pool.alloc(16);
    pool.dealloc(a);
    pool.dealloc(b);
    pool.dealloc(c);

    d = pool.alloc(8);
    e = pool.alloc(16);
    pool.dealloc(d);
    pool.dealloc(e);

    result &= (d == a && e == b);
  }

  return result;
}

END_SUITE()
