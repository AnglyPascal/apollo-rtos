#pragma once

#include "core/memory.h"
#include "core/types.h"
#include "utility/circular_buffer.h"

inline constexpr size_t N_PROCS = 16;

template <size_t chan_len>
using chan_t = circular_buffer<pid_t, chan_len>;

namespace sched
{
template <size_t chan_len>
void wait(chan_t<chan_len> &chan);

template <size_t chan_len>
void notify(chan_t<chan_len> &chan);
} // namespace sched

struct barrier_t {
  const size_t num;
  size_t arrived = 0;
  chan_t<1> chan;

  barrier_t(size_t num) : num{num} {}

  void acquire()
  {
    if (++arrived == num)
      sched::wait(chan);
  }

  void release()
  {
    if (--arrived == 0)
      sched::notify(chan);
  }
};

struct proc_def_t {
  const string name;
  const priority_lev_t priority;
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

pid_t _reg_proc_impl(const proc_def_t *proc_def, void *param);

template <typename T>
pid_t reg_proc(const proc_def_t *proc_def, T &&t)
{
  using U = remove_ref_cv_t<T>;
  if constexpr (is_any_of<U, decltype(nullptr), void *>)
    return _reg_proc_impl(proc_def, (void *)t);
  else
    return _reg_proc_impl(proc_def, (void *)kmem::knew<U>(std::forward<T>(t)));
}

void incr_priority(pid_t pid, int32_t priority);
inline void incr_priority(int32_t priority)
{
  return incr_priority(curr_proc::pid(), priority);
}

void decr_priority(int32_t priority);

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

void notify_exit(pid_t pid, barrier_t *bar);

template <typename... Args>
  requires(std::same_as<remove_ref_cv_t<Args>, pid_t> && ...)
void wait(const Args &...args)
{
  barrier_t bar{sizeof...(args)};
  auto lam = [&](auto arg) {
    notify_exit(arg, &bar);
    bar.acquire();
  };
  (lam(args), ...);
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

#define SERVICE(...) DEF_MACRO(.services, ##__VA_ARGS__)
#define STARTUP_PROC(...) DEF_MACRO(.startups, ##__VA_ARGS__)
#define APP(...) DEF_MACRO(.apps, ##__VA_ARGS__)
#define PROC(...) DEF_MACRO(.data, ##__VA_ARGS__)

#define REG_PROC(name, ...) sched::reg_proc(&PROC_DEF(name), ##__VA_ARGS__)
