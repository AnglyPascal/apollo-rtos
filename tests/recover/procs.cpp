#include "core/memory.h"
#include "core/recover.h"
#include "core/sched.h"
#include "utility/args.h"

namespace
{
PROC(func, MID1, 128, i)
{
  kprintf("f%d\r\n", *(int *)i);
  while (1) {
    sched::sleep(2000);
  }
}

PROC(guarded_func, HIGH4, 128, p)
{
  auto i = *(int *)p;
  recover::guard_proc guard{POWER_OFF, i};
  kprintf("g%d\r\n", i);
  while (1) {
    sched::sleep(2000);
  }
}

PROC(nested_func, MID4, 128, p)
{
  auto i = *(int *)p;
  recover::guard_proc guard{POWER_OFF, i};
  kprintf("n%d\r\n", i);
  sched::reg_proc(&DEF(func), p);
  kprintf("nf%d\r\n", i);
  while (1) {
    sched::sleep(1000);
  }
}
} // namespace

namespace sched
{
void setup_procs(void)
{
  for (int i = 0; i < 5; i++) {
    reg_proc(&DEF(func), kmem::knew<int>(i));
  }

  for (int i = 0; i < 5; i++) {
    reg_proc(&DEF(guarded_func), kmem::knew<int>(i));
  }

  for (int i = 0; i < 3; i++) {
    reg_proc(&DEF(nested_func), kmem::knew<int>(i));
  }
}
} // namespace sched
