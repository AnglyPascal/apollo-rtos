#pragma once

#include "core/types.h"
#include "drivers/serial.h"
#include "utility/lib.h"

enum debug_t {
  FATAL,
  ERROR,
  WARN,
  INFO,
  DEBUG,
  TRACE,
};

#ifndef NDEBUG
constexpr auto DEBUG_LEV = DEBUG;
#else
constexpr auto DEBUG_LEV = WARN;
#endif

template <debug_t level, typename... Args>
void debug(const char *fmt, Args... args)
{
  if constexpr (level <= DEBUG_LEV) {
    if (level <= ERROR)
      serial::kprintf(fmt, args...);
    else
      serial::printf(fmt, args...);
  }
}

__extern_C__
void spin(void);

template <debug_t lev = TRACE>
__always_inline__
inline void __assert(bool ex, const char *src, const char *func,
                     const char *file, int line)
{
#ifndef NDEBUG
  if (!ex) {
    debug<lev>("\r\nassertion failed ``%s``, in %s, at %s:%d\r\n", src, func,
               file, line);
    return trigger_hardfault();
  }
#endif
}

template <debug_t lev = TRACE, typename... Args>
__always_inline__
inline void __assert(bool ex, const char *src, const char *func,
                     const char *file, int line, const char *fmt, Args... args)
{
#ifndef NDEBUG
  if (!ex) {
    debug<lev>("\r\nassertion failed ``%s``, in %s, at %s:%d\r\n", src, func,
               file, line);
    debug<lev>(fmt, args...);
    return trigger_hardfault();
  }
#endif
}

#define assert(EX, ...)                                                        \
  __assert((EX), #EX, __func__, __FILE__, __LINE__, ##__VA_ARGS__)

#define assert_dump(EX, ...)                                                        \
  __assert<ERROR>((EX), #EX, __func__, __FILE__, __LINE__, ##__VA_ARGS__)
