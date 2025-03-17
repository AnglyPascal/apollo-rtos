#pragma once

#include "core/types.h"
#include "drivers/serial.h"
#include "utility/lib.h"

enum debug_t {
  FATAL = 0,
  H_RESET = FATAL,

  ERROR = 1,
  S_RESET = ERROR,

  WARN = 2,
  TERM = WARN,

  INFO = 3,
  DEBUG = 4,
  TRACE = 5,
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
    if constexpr (level <= ERROR)
      serial::kprintf(fmt, args...);
    else
      serial::printf(fmt, args...);
  }
}

void trigger_term(void);

template <debug_t lev, typename... Args>
  requires(lev <= WARN)
__always_inline__
inline void __assert(bool ex, Args... args)
{
  if (ex)
    return;

  auto dump = []<typename... Ts>(const char *src, const char *func, Ts... ts) {
    debug<ERROR>("\r\nassertion failed ``%s``, in %s\r\n", src, func);
    if constexpr (sizeof...(ts) != 0)
      debug<ERROR>(ts...);
  };

  if constexpr (lev == H_RESET) {
    if constexpr (sizeof...(args) != 0)
      dump(args...);
    return trigger_hardfault();
  }

  if constexpr (lev == S_RESET)
    return trigger_reset();

  if constexpr (lev == TERM)
    return trigger_term();
}

#define assert(EX, LEV, ...) __assert<LEV>((EX))

#define assert_dump(EX, LEV, ...)                                              \
  __assert<LEV>((EX), #EX, __func__, ##__VA_ARGS__)

