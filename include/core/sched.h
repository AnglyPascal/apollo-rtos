#pragma once

#include "core/types.h"
#include "utility/circular_buffer.h"

inline constexpr size_t N_PROCS = 16;
struct proc_t;

inline constexpr size_t N_PROCS_WAIT = 4;
template <size_t chan_len = N_PROCS_WAIT>
struct chan_t {
  pid_t dequeue() { return pids.dequeue(); }
  void enqueue(pid_t pid) { pids.enqueue(pid); }
  bool empty() const { return pids.empty(); }

private:
  circular_buffer<pid_t, chan_len> pids;
};

struct proc_def_t {
  string name;
  priority_t priority;
  size_t stk_sz;
  runnable_t func;
};

namespace curr_proc
{
pid_t pid();
size_t rec_entry_id();
string name();
} // namespace curr_proc

namespace sched
{

void incr_priority(proc_t *proc, priority_t priority);
void decr_priority(priority_t priority);

inline constexpr time_t invoke_interval = 2048;

bool needs_swap();
void change_proc();

proc_t *reg_proc(proc_def_t *proc_def, void *param);

void init();

void sleep(time_t period);

void sleep();
void wakeup(pid_t pid);

template <size_t chan_len>
void wait(chan_t<chan_len> *chan)
{
  chan->enqueue(curr_proc::pid());
  sleep();
}

template <size_t chan_len>
void notify(chan_t<chan_len> *chan)
{
  if (chan->empty())
    return;

  auto pid = chan->dequeue();
  wakeup(pid);
}

void trace();

void transfer_param(void *param);

extern volatile time_t last_checked;

} // namespace sched
