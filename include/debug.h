#pragma once

#include "lib.h"
#include "serial.h"

enum debug_t {
  FATAL,
  ERROR,
  WARN,
  INFO,
  DEBUG,
  TRACE,
};

constexpr auto DEBUG_LEV =
#ifndef NDEBUG
    WARN;
#else
    ERROR;
#endif

template <debug_t level, typename... Args>
void debug(Args... args)
{
  if constexpr (level < DEBUG_LEV) {
    printf(args...);
  }
}

inline void assert(bool c)
{
  if constexpr (DEBUG_LEV >= DEBUG) {
    if (!c)
      debug<FATAL>("assertion failed in %s, in %s:%s\n", __func__, __FILE__);
  }
}

