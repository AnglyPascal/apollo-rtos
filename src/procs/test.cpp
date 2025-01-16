#include "circular_buffer.h"
#include "sched.h"

__extern_C__
void trigger_reset();

namespace procs
{

void *test_proc(void *param)
{
  int n = 3;
  while (n-- > 0)
    ;

  circular_buffer<int, 10> buf;
  buf.enqueue(10);
  buf.enqueue(5);
  buf.enqueue(1);
  for (auto i : buf) {
    debug<DEBUG>("%d, ", i);
  }
  debug<DEBUG>("\r\n");
  buf.dequeue();
  for (auto i : buf) {
    debug<DEBUG>("%d, ", i);
  }
  debug<DEBUG>("\r\n");

  sched::decr_priority(5);

  while (1) {
    int m = 10;
    while (m-- > 0) {
      n = 100000000;
      while (n-- > 0)
        ;
    }

    sched::sleep(50);
  }

  return param;
}

} // namespace procs
