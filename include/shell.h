#pragma once

#include "sched.h"
#include "types.h"

namespace shell
{

constexpr size_t args_len = 64;

struct buffer {
  char str[args_len] = {'\0'};
  size_t sz = 0;

  char *args = nullptr;
  bool run_bg = false;

  char &operator[](size_t i) { return str[i]; }

  void push(char c)
  {
    if (sz < args_len)
      str[sz++] = c;
  }

  void pop()
  {
    if (sz > 0)
      sz--;
  }

  void reset() { *this = buffer{}; }
};

void init();

proc_def_t *match_cmd(string cmd);

} // namespace shell
