#pragma once

#include "procs.h"
#include "types.h"

#include <cstdint>

namespace sched
{

void incr_priority(proc_t *proc, priority_t priority);
void decr_priority(priority_t priority);

inline constexpr time_t invoke_interval = 512;
uint8_t *invoke(uint8_t *old_stack, time_t millis);

proc_t *reg_proc(string name, priority_t priority, uint32_t stack_sz,
                 runnable_t func, void *param);

void init();

void sleep(time_t period);
void sleep(time_t period, void (*alarm)(void *), void *param);

__extern_C__
uint8_t *cxt_switch(uint8_t *stk_ptr);

void trace();

} // namespace sched
