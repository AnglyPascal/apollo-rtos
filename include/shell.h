#pragma once

#include "sched.h"
#include "types.h"

namespace shell
{

constexpr size_t args_len = 64;

struct cmd_t {
  string cmd;

  priority_t priority;
  size_t stk_sz;

  runnable_t func;
};

void proc(void *);

void init();

cmd_t *match_cmd(string cmd);

inline void *default_parse(string)
{
  return nullptr;
}

} // namespace shell
