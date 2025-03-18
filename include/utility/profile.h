#pragma once

#include "core/types.h"

namespace profile
{

struct entry_t {
  const char *func_name = nullptr;
  uint32_t ticks = 0;
};

inline constexpr size_t N_PROFILES = 8;

class profile_guard
{
  size_t entry_id;

public:
  profile_guard(size_t entry_id, const char *name);
  ~profile_guard();
};

void trace();

#define PROFILE_THIS() profile::profile_guard ___guard{__COUNTER__, __func__};

} // namespace profile
