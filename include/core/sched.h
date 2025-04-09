#pragma once

#include "core/types.h"
#include "utility/circular_buffer.h"

inline constexpr size_t N_PROCS = 16;

template <size_t chan_len>
struct chan_t : public circular_buffer<pid_t, chan_len> {
};

struct proc_def_t {
  const string name;
  const priority_t priority;
  const size_t stk_sz;
  const runnable_t func;

  mutable time_t ticks = 0;
  void tick() const { ticks++; }
};

namespace curr_proc
{
pid_t pid();
string name();
bool term_req();
bool set_up();
} // namespace curr_proc

namespace sched
{
void init();

pid_t reg_proc(const proc_def_t *proc_def, void *param);
void incr_priority(pid_t pid, priority_t priority);
void decr_priority(priority_t priority);

inline constexpr time_t invoke_interval = 2048;
extern volatile time_t last_checked;
bool needs_swap();

void sleep(time_t period = 0);
void wakeup(pid_t pid);

template <size_t chan_len>
void wait(chan_t<chan_len> &chan)
{
  intr_guard guard;
  chan.enqueue(curr_proc::pid());
  sleep();
}

template <size_t chan_len>
void notify(chan_t<chan_len> &chan)
{
  intr_guard guard;
  if (chan.empty())
    return;
  wakeup(chan.dequeue());
}

template <size_t chan_len>
void notify_all(chan_t<chan_len> &chan)
{
  intr_guard guard;
  while (!chan.empty())
    wakeup(chan.dequeue());
}

void trace();
} // namespace sched

#define PROC_DEF(name) __##name##_def
#define PROC_FUNC(name) __##name##_func

#define DEF_MACRO(sec, name, priority, stk_sz, param)                          \
  void PROC_FUNC(name)(void *);                                                \
  const proc_def_t __attribute__((section(#sec), __used__)) PROC_DEF(name){    \
      #name, priority, stk_sz, PROC_FUNC(name)};                               \
  void PROC_FUNC(name)(void *param)

#define STARTUP(...) DEF_MACRO(.startup, ##__VA_ARGS__)
#define PROC(...) DEF_MACRO(.procs, ##__VA_ARGS__)
#define APP(...) DEF_MACRO(.apps, ##__VA_ARGS__)
#define PROC_MANUAL(...) DEF_MACRO(.data, ##__VA_ARGS__)

