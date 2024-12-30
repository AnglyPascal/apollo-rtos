#pragma once

#include "timer.h"
#include "types.h"

namespace waitlist
{

constexpr time_t update_interval = 1 << 4;

void reg(time_t interval, runnable_t func, void *param = nullptr);
void run();

}; // namespace waitlist
