#pragma once

#include <cstdint>
#include <limits>
#include <utility>

// arm gcc specific attributes
#define __always_inline__ __attribute__((always_inline))
#define __noinline__ __attribute__((noinline))
#define __extern_C__ extern "C"

template <typename T>
constexpr T MAX = std::numeric_limits<T>::max();

using size_t = std::size_t;
using runnable_t = void (*)(void *);

using pid_t = uint8_t;
inline constexpr pid_t null_pid = MAX<pid_t>;

using priority_t = int32_t;

enum priorities : priority_t {
  EMPTY,

  IDLE = 1,

  LOW1 = 2,
  LOW2,
  LOW3,
  LOW4,

  MID1 = 16,
  MID2,
  MID3,
  MID4,

  HIGH1 = 64,
  HIGH2,
  HIGH3,
  HIGH4,

  URGENT1 = 128,
  URGENT2,
  URGENT3,

  HIGHEST = MAX<priority_t>,
};

using word_t = uint32_t;
using byte_t = std::byte;

using time_t = uint32_t;

struct string {
  const char *str;

  constexpr string() : str{nullptr} {}
  constexpr string(const string &other) : str(other.str) {}
  constexpr string(const char *_str) : str{_str} {}

  bool operator==(const string &other) const
  {
    auto lhs = str, rhs = other.str;

    if (!lhs && !rhs)
      return true;

    if (!lhs || !rhs)
      return false;

    while (*lhs != '\0' && *rhs != '\0' && *lhs == *rhs) {
      lhs++;
      rhs++;
    }

    return *lhs == '\0' && *rhs == '\0';
  }
};

template <typename T>
constexpr T max(T t, T s)
{
  return t > s ? t : s;
}

template <typename T>
constexpr T min(T t, T s)
{
  return t < s ? t : s;
}

constexpr auto abs(auto t) {
  return t > 0 ? t : -t;
}

inline void *operator new(size_t, void *where) { return where; }

inline constexpr size_t pg_sz = 1024;

#define MEMORY_FENCE() asm volatile("" ::: "memory")
