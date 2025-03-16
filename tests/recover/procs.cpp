#include "core/recover.h"
#include "core/sched.h"
#include "core/shell.h"

namespace
{
void func(void *i)
{
  while (1) {
    printf("func %d\r\n", i);
    sched::sleep(2000);
  }
}

void guarded_func(void *i)
{
  recover::guard_proc guard{POWER_OFF, i};
  while (1) {
    printf("guarded func %d\r\n", i);
    sched::sleep(2000);
  }
}

proc_def_t func_def = {"func", MID1, 128, func};
proc_def_t guarded_func_def = {"guarded_func", MID4, 128, guarded_func};

void nested_func(void *i)
{
  recover::guard_proc guard{POWER_OFF, i};
  sched::reg_proc(&func_def, i);
  while (1) {
    printf("nested func %d\r\n", i);
    sched::sleep(1000);
  }
}

proc_def_t nested_func_def = {"nested_func", HIGH1, 128, nested_func};
} // namespace

namespace sched
{
void setup_procs(void)
{
  for (int i = 0; i < 5; i++) {
    reg_proc(&func_def, (void *)i);
  }

  for (int i = 0; i < 5; i++) {
    reg_proc(&guarded_func_def, (void *)i);
  }

  for (int i = 0; i < 3; i++) {
    reg_proc(&nested_func_def, (void *)i);
  }
}
} // namespace sched

namespace shell
{
extern proc_def_t echo_cmd, trace_cmd, heart_cmd;

namespace
{
proc_def_t *cmds[] = {
    &echo_cmd,
    &trace_cmd,
    &heart_cmd,
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
