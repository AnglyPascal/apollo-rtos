#pragma once

#include "core/sched.h"
#include "core/types.h"
#include "utility/debug.h"

namespace shell
{
constexpr size_t args_len = 64;

struct args_buffer_t {
  size_t sz = 0;
  char str[args_len] = {'\0'};

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

  void reset() { sz = 0; }
};

struct args_t {
  char str[args_len];
  bool run_bg;
};

} // namespace shell
