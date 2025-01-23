#include "shell.h"

namespace shell
{

extern proc_def_t echo_cmd, trace_cmd, heart_cmd, show_cmd, calc_cmd, accel_cmd;

namespace
{
proc_def_t *cmds[] = {&echo_cmd, &trace_cmd, &heart_cmd,
                      &show_cmd, &calc_cmd,  &accel_cmd};
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
