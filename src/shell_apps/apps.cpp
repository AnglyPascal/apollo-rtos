#include "shell.h"

namespace shell
{

extern cmd_t echo_cmd, trace_cmd, heart_cmd, show_cmd;

namespace
{
cmd_t *cmds[] = {&echo_cmd, &trace_cmd, &heart_cmd, &show_cmd};
}

cmd_t *match_cmd(string cmd_str)
{
  for (auto cmd : cmds) {
    if (cmd->cmd == cmd_str)
      return cmd;
  }
  return nullptr;
}

} // namespace shell
