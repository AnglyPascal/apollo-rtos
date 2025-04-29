#include "core/memory.h"
#include "core/sched.h"
#include "core/test.h"
#include "utility/debug.h"

BEGIN_SUITE(malloc)

void *a, *b, *c, *d;
bool result = true;

// Basic malloc/free functionality within a single process
PROC(malloc_free_basic, MID1, 8, param)
{
  a = heap::malloc(16);
  b = heap::malloc(32);
  heap::free(a);
  c = heap::malloc(16);

  // Store results in process-safe memory
  result &= (a == c) && (a != b);
}

SYS_TEST(basic_malloc_free)
{
  result = true;
  auto p = REG_PROC(malloc_free_basic, nullptr);
  sched::wait(p);
  return result;
}

// Verify cleanup after process termination
PROC(proc_cleanup_test, MID2, 8, param)
{
  a = heap::malloc(64);
  b = heap::malloc(128);
  // Intentionally don't free 'a' and 'b'
}

SYS_TEST(cleanup_after_process)
{
  auto p = REG_PROC(proc_cleanup_test, nullptr);
  sched::wait(p);

  // Should reuse memory from cleaned-up process
  c = heap::malloc(64);
  d = heap::malloc(128);

  return (c == a) && (d == b);
}

// Test null handling and zero-size allocation
PROC(null_handling_test, MID3, 8, param)
{
  heap::free(nullptr); // Shouldn't crash
  void *zero_alloc = heap::malloc(0);
  result &= (zero_alloc == nullptr);
}

SYS_TEST(null_and_zero_handling)
{
  result = true;
  auto p = REG_PROC(null_handling_test, nullptr);
  sched::wait(p);
  return result;
}

// Verify isolation between processes
PROC(proc1_alloc, MID4, 8, param)
{
  a = heap::malloc(256);
  sched::sleep(5);
}

PROC(proc2_alloc, MID3, 8, param)
{
  b = heap::malloc(256);
  sched::sleep(5);
}

SYS_TEST(process_isolation)
{
  auto p1 = REG_PROC(proc1_alloc, nullptr);
  auto p2 = REG_PROC(proc2_alloc, nullptr);
  sched::wait(p1, p2);

  // Processes should have different memory regions
  return a != b;
}

// Test complex allocation pattern with cleanup
PROC(complex_alloc_pattern, MID3, 8, param)
{
  void *chunks[4];
  chunks[0] = heap::malloc(16);
  chunks[1] = heap::malloc(32);
  heap::free(chunks[0]);
  chunks[2] = heap::malloc(16); // Should reuse first chunk
  chunks[3] = heap::malloc(64);

  // Store results in shared memory
  result &= (chunks[0] == chunks[2]) && (chunks[1] != chunks[3]);
}

SYS_TEST(complex_pattern_with_cleanup)
{
  result = true;
  auto p = REG_PROC(complex_alloc_pattern, nullptr);
  sched::wait(p);
  return result;
}

END_SUITE()
