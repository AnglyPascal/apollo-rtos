#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

// arm gcc specific attributes
#define __always_inline__ __attribute__((always_inline))
#define __noinline__ __attribute__((noinline))
#define __extern_C__ extern "C"
#define __naked__ __attribute__((naked))

template <typename T>
constexpr T MAX = std::numeric_limits<T>::max();

using size_t = std::size_t;
using runnable_t = void (*)(void *);

using word_t = uint32_t;
using byte_t = std::byte;

constexpr uint32_t pow(int base, int p)
{
  while (p-- > 0)
    base *= base;
  return base;
}

struct rational_t {
  int32_t num;
  uint32_t denom;

  static constexpr uint8_t precision = 3;
  static constexpr uint32_t mult = pow(10, precision);

  rational_t(float f) : num{(int32_t)(f * mult)}, denom{mult} {}
  rational_t(int32_t num, uint32_t denom) : num{num}, denom{denom} {}
};

using time_t = uint32_t;

using pid_t = uint8_t;
inline constexpr pid_t null_pid = MAX<pid_t>;

using priority_t = int32_t;
enum : priority_t {
  EMPTY = 0,

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

enum {
  O_READ = 0,
  O_CREATE = 1 << 0,
  O_WRITE = 1 << 1,
  O_CHAR_FILE = 1 << 2,
  O_PERM = 1 << 3,
  O_SHARED = 1 << 4,
};
using fn_t = uint8_t;
inline constexpr fn_t null_fn = MAX<fn_t>;

enum file_type_t {
  CHAR = 1 << 0,
  BIN = 1 << 1,
};

using std::pair;

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

constexpr auto abs(auto t) { return t > 0 ? t : -t; }

inline void *operator new(size_t, void *where) { return where; }

inline constexpr size_t pg_sz = 1024;

#define MEMORY_FENCE() asm volatile("" ::: "memory")

inline void trigger_hardfault() { asm("udf #0"); }

inline void trigger_reset()
{
  asm("ldr r0, =0xE000ED0C");
  asm("ldr r1, =0x05FA0004");
  asm("str r1, [r0]");
}

constexpr size_t roundup(size_t sz, size_t align)
{
  return (sz + align - 1) & ~(align - 1);
}

#define SECTION_ADDR(name)                                                     \
  __extern_C__ byte_t __##name##_load[], __##name##_start[], __##name##_end[]

#define SECTION_INIT(name)                                                     \
  _memcpy(__##name##_start, __##name##_load, __##name##_end - __##name##_start)

#define SECTION_ITER(name, type, var)                                          \
  type *var = (type *)__##name##_start; var < (type *)__##name##_end; var++

