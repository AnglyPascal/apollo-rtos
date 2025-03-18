#pragma once

#include "core/types.h"

namespace timer
{

void init();

extern volatile time_t MILLIS;

__always_inline__
inline time_t now()
{
  return MILLIS;
}

extern volatile time_t debug_ticks;

__always_inline__
inline uint32_t total_ticks()
{
  return (uint32_t)debug_ticks;
}

} // namespace timer
