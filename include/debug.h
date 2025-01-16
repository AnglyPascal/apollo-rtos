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

constexpr auto DEBUG_LEV = DEBUG;
constexpr auto ASSERT_EN = DEBUG_LEV >= DEBUG;

template <debug_t level, typename... Args>
void debug(Args... args)
{
  if constexpr (level <= DEBUG_LEV) {
    printf(args...);
  }
}

__extern_C__
void spin(void);

inline void __assert(bool ex, const char *src, const char *func,
                     const char *file, int line)
{
  if constexpr (ASSERT_EN)
    if (!ex) {
      debug<FATAL>("assertion failed ``%s``, in %s, at %s:%d\r\n", src, func,
                   file, line);
      asm("udf #0");
    }
}

#define assert(EX) __assert((EX), #EX, __func__, __FILE__, __LINE__)
