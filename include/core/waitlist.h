#pragma once

#include "core/types.h"

namespace waitlist
{
inline constexpr time_t update_interval = 1 << 3;

void reg(const char *name, time_t interval, void (*func)(void *),
         void *param = nullptr);
}; // namespace waitlist
