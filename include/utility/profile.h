#pragma once

#include "core/types.h"
#include "drivers/timer.h"
#include "utility/bitset.h"
#include "utility/debug.h"

namespace profile
{

struct entry_t {
  const char *func_name = nullptr;
  uint32_t ticks = 0;
};

inline constexpr size_t N_PROFILES = 8;

inline entry_t tbl[N_PROFILES] = {};
inline bitset<N_PROFILES> set;

template <size_t ID>
struct __profile_guard {
  __profile_guard(const char *name)
  {
    tbl[ID].func_name = name;
    set.insert(ID);
  }

  ~__profile_guard() { set.erase(ID); }
};

inline void tick()
{
  for (auto id : set) {
    tbl[id].ticks++;
  }
}

#define PROFILE_THIS(ID)                                                       \
  profile::__profile_guard<ID> __##__func__##_guard{__func__};

} // namespace profile
