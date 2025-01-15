#include "sched.h"

namespace procs
{

void *proc1(void *param)
{
  int n = 5;
  while (n-- > 0)
    ;
  return param;
}

void *proc2(void *param)
{
  int n = 3;
  while (n-- > 0)
    ;
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
