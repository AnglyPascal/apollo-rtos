#pragma once

#include "timer.h"
#include "types.h"

#include <cstdint>

using pid_t = uint8_t;
using priority_t = int32_t;

namespace sched
{

void decr_priority(priority_t priority);

inline constexpr time_t invoke_interval = 512;
void *invoke(void *old_stack, time_t millis);

struct proc_t;
proc_t *reg_proc(string name, priority_t priority, uint32_t stack_sz,
                 runnable_t func, void *param);

void init();

void sleep(time_t period);
void sleep(time_t period, void (*alarm)(void *), void *param);

extern "C" void *cxt_switch(void *stk_ptr);

void trace();

void incr_priority(proc_t *, priority_t);

} // namespace sched

