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
void debug(const char *fmt, Args... args)
{
  if constexpr (level <= DEBUG_LEV) {
    printf(fmt, args...);
  }
}

__extern_C__
void spin(void);

__always_inline__
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

template <typename... Args>
__always_inline__
inline void __assert(bool ex, const char *src, const char *func,
                     const char *file, int line, const char *fmt, Args... args)
{
  if constexpr (ASSERT_EN)
    if (!ex) {
      debug<FATAL>("assertion failed ``%s``, in %s, at %s:%d\r\n", src, func,
                   file, line);
      debug<FATAL>(fmt, args...);
      asm("udf #0");
    }
}

#define assert(EX, ...)                                                        \
  __assert((EX), #EX, __func__, __FILE__, __LINE__, ##__VA_ARGS__)

