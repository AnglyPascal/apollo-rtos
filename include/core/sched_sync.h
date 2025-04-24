#pragma once

#include "core/types.h"
#include "utility/circular_buffer.h"

namespace curr_proc
{
pid_t pid();
}

template <size_t chan_len>
using chan_t = circular_buffer<pid_t, chan_len>;

namespace sched
{
// returns true if was woken up by the alarm
bool sleep(time_t timeout = 0);
void wakeup(pid_t pid);
} // namespace sched

namespace sched
{
// returns false if woken up by timeout
template <size_t chan_len>
bool wait(chan_t<chan_len> &chan, time_t timeout = 0)
{
  intr_guard guard;
  chan.enqueue(curr_proc::pid());
  return !sleep(timeout);
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
} // namespace sched

class barrier_t
{
private:
  const size_t num;
  size_t arrived = 0;

  chan_t<1> chan;
  time_t timeout = 0;

public:
  barrier_t(size_t num, time_t timeout = 0) : num{num}, timeout{timeout} {}

  // returns false if woken up by timeout
  bool acquire()
  {
    if (++arrived == num)
      return sched::wait(chan, timeout);
    return true;
  }

  void release()
  {
    if (--arrived == 0)
      sched::notify(chan);
  }
};

namespace sched
{
void notify_exit(pid_t pid, barrier_t *bar);

// returns false if was woken up prematuredly
template <typename... Args>
  requires(std::same_as<remove_ref_cv_t<Args>, pid_t> && ...)
bool wait(time_t timeout, const Args &...args)
{
  barrier_t bar{sizeof...(args), timeout};
  auto lam = [&](auto arg) {
    notify_exit(arg, &bar);
    return bar.acquire();
  };
  return (lam(args) && ...);
}

template <typename... Args>
  requires(std::same_as<remove_ref_cv_t<Args>, pid_t> && ...)
void wait(const Args &...args)
{
  wait(0, args...);
}
} // namespace sched
