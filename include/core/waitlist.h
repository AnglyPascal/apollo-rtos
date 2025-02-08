#pragma once

#include "core/types.h"
#include "drivers/timer.h"

namespace waitlist
{

constexpr time_t update_interval = 1 << 3;

void reg(string name, time_t interval, void (*func)(void *),
         void *param = nullptr);

void run();

void trace();

}; // namespace waitlist
