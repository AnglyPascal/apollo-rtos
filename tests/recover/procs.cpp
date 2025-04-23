#include "core/memory.h"
#include "core/recover.h"
#include "core/sched.h"
#include "utility/args.h"

namespace
{
PROC(func, MID1, 128, i)
{
  kprintf("f%d\r\n", *(int *)i);
  while (1)
    sched::sleep(2000);
}

PROC(guarded_func, HIGH4, 128, p)
{
  auto i = *(int *)p;
  recover::guard_proc guard{POWER_OFF, i};

  kprintf("g%d\r\n", i);
  while (1)
    sched::sleep(2000);
}

PROC(nested_func, MID4, 128, p)
{
  auto i = *(int *)p;
  recover::guard_proc guard{POWER_OFF, i};
  kprintf("n%d\r\n", i);

  REG_PROC(func, (void *)p);

  kprintf("nf%d\r\n", i);
  while (1)
    sched::sleep(1000);
}
} // namespace

namespace sched
{
void setup_procs(void)
{
  for (int i = 0; i < 5; i++)
    REG_PROC(func, i);

  for (int i = 0; i < 5; i++)
    REG_PROC(guarded_func, i);

  for (int i = 0; i < 3; i++)
    REG_PROC(nested_func, i);
}
} // namespace sched
