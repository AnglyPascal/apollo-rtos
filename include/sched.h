#pragma once

#include "types.h"
#include <cstdint>

inline constexpr size_t N_PROCS = 16;
struct proc_t;

struct proc_def_t {
  string name;
  priority_t priority;
  size_t stk_sz;
  runnable_t func;
  void *param;
};

namespace sched
{

void incr_priority(proc_t *proc, priority_t priority);
void decr_priority(priority_t priority);

inline constexpr time_t invoke_interval = 2048;

bool needs_swap();
void change_proc();

proc_t *reg_proc(proc_def_t *proc_def);
proc_t *reg_proc(string name, priority_t priority, size_t stk_sz,
                 runnable_t func, void *param);

void init();

void sleep(time_t period);
void sleep(time_t period, void (*alarm)(void *), void *param);

void trace();

pid_t curr_pid();

extern volatile time_t last_checked;

} // namespace sched
