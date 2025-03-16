#include "core/memory.h"
#include "core/recover.h"
#include "core/sched.h"
#include "core/shell.h"

namespace
{
void func(void *i)
{
  kprintf("f%d\r\n", *(int *)i);
  while (1) {
    sched::sleep(2000);
  }
}

void guarded_func(void *p)
{
  auto i = *(int *)p;
  recover::guard_proc guard{POWER_OFF, i};
  kprintf("g%d\r\n", i);
  while (1) {
    sched::sleep(2000);
  }
}

proc_def_t func_def = {"f", MID1, 128, func};
proc_def_t guarded_func_def = {"g", HIGH4, 128, guarded_func};

void nested_func(void *p)
{
  auto i = *(int *)p;
  recover::guard_proc guard{POWER_OFF, i};
  kprintf("n%d\r\n", i);
  sched::reg_proc(&func_def, p);
  kprintf("nf%d\r\n", i);
  while (1) {
    sched::sleep(1000);
  }
}

proc_def_t nested_func_def = {"n", MID4, 128, nested_func};
} // namespace

namespace sched
{
void setup_procs(void)
{
  for (int i = 0; i < 5; i++) {
    reg_proc(&func_def, kmem::knew<int>(i));
  }

  for (int i = 0; i < 5; i++) {
    reg_proc(&guarded_func_def, kmem::knew<int>(i));
  }

  for (int i = 0; i < 3; i++) {
    reg_proc(&nested_func_def, kmem::knew<int>(i));
  }
}
} // namespace sched

namespace shell
{
extern proc_def_t echo_cmd, trace_cmd, heart_cmd, pkill_cmd;

namespace
{
proc_def_t *cmds[] = {
    &echo_cmd,
    &trace_cmd,
    &heart_cmd,
    &pkill_cmd,
};
}

proc_def_t *match_cmd(string cmd_str)
{
  for (auto cmd : cmds) {
    if (cmd->name == cmd_str)
      return cmd;
  }
  return nullptr;
}
} // namespace shell
