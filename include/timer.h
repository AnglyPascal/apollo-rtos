#pragma once

#include "types.h"

namespace timer
{

void init();

extern volatile time_t MILLIS;

__always_inline__
inline time_t now()
{
  return MILLIS;
}

} // namespace timer
