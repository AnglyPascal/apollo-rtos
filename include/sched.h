#pragma once

#include "types.h"

inline constexpr size_t N_PROCS = 16;
struct proc_t;

struct chan_t {
  pid_t pid = N_PROCS;
  volatile uint32_t *event = nullptr;

  chan_t() {}
  chan_t(volatile uint32_t *event) : pid(N_PROCS), event(event) {}
};

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

void wait(chan_t *);
void notify(chan_t *);

void trace();

void transfer_param(void *param);

extern volatile time_t last_checked;

} // namespace sched

namespace curr_proc
{
pid_t pid();
size_t rec_entry_id();
string name();
} // namespace curr_proc
