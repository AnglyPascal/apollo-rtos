#pragma once

#include "timer.h"
#include "types.h"

namespace waitlist
{

constexpr time_t update_interval = 1 << 4;

void reg(string name, time_t interval, void (*func)(void *),
         void *param = nullptr);

void run();

void trace();

}; // namespace waitlist
