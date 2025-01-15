#pragma once

#include "char_buffer.h"
#include "sched.h"
#include "types.h"

namespace shell
{

constexpr size_t args_len = 64;
using buffer = char_buffer<args_len>;

struct cmd_t {
  string cmd;

  priority_t priority;
  size_t stk_sz;

  runnable_t func;
};

void init();

cmd_t *match_cmd(string cmd);

} // namespace shell
