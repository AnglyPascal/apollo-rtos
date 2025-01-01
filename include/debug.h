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
    DEBUG;
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
