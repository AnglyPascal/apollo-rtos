#pragma once

#include "core/types.h"
#include "utility/format.h"

inline constexpr uint32_t HARDFAULT_MAGIC = 0xDEADDAAD;

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
    if constexpr (level <= ERROR) {
      kprintf(BOLD RED);
      kprintf(fmt, args...);
      kprintf(DEFAULT);
    } else {
      if constexpr (level == WARN)
        printf(YELLOW);

      printf(fmt, args...);
      printf(DEFAULT);
    }
  }
}

void trigger_term(void);

template <debug_t lev, typename... Args>
  requires(lev <= WARN)
void __assert(bool ex, const char *file, uint32_t line, Args &&...args)
{
  if (ex)
    return;

  if constexpr (lev == H_RESET) {
    if constexpr (sizeof...(args) != 0)
      debug<ERROR>(std::forward<Args>(args)...);

    asm volatile("mov r0, %[input_file]\n"
                 "mov r1, %[input_line]\n"
                 "mov r2, %[magic]\n"
                 :
                 : [input_file] "r"(file), [input_line] "r"(line),
                   [magic] "r"(HARDFAULT_MAGIC)
                 : "r0", "r1");

    return trigger_hardfault();
  }

  if constexpr (lev == S_RESET)
    return trigger_reset();

  if constexpr (lev == TERM)
    return trigger_term();
}

#define assert(EX, LEV, ...) __assert<LEV>((EX), __FILENAME__, __LINE__)

#define assert_dump(EX, LEV, ...)                                              \
  __assert<LEV>((EX), __FILENAME__, __LINE__, ##__VA_ARGS__)

#define halt() assert(false, H_RESET)
