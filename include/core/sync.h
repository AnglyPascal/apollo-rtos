#pragma once

#include "core/types.h"
#include "utility/circular_buffer.h"

namespace curr_proc
{
pid_t pid();
}

template <size_t chan_len>
class chan_t : public circular_buffer<pid_t, chan_len>
{
};

template <>
class chan_t<1>
{
  pid_t pid = null_pid;

public:
  void enqueue(pid_t _pid) { pid = _pid; }
  pid_t dequeue()
  {
    auto _pid = pid;
    pid = null_pid;
    return _pid;
  }

  bool empty() const { return pid == null_pid; }
  bool is_full() const { return !empty(); }
  size_t size() const { return (size_t)is_full(); }
  constexpr size_t capacity() const { return 1; }
};

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

  if (chan.is_full()) {
    debug<ERROR>("channel is full\r\n");
    return false;
  }

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
  size_t num;
  uint32_t to_arrive = 0;

  chan_t<1> chan;
  time_t timeout = 0;

  static_assert(MAX<uint32_t> >= (1u << (N_PROCS - 1)));

public:
  barrier_t(size_t num, time_t timeout = 0) : num{num}, timeout{timeout} {}

  // returns false if woken up by timeout
  bool acquire(pid_t pid)
  {
    to_arrive |= (1u << pid);
    if (--num == 0)
      return sched::wait(chan, timeout);
    return true;
  }

  void release(pid_t pid)
  {
    to_arrive &= ~(1u << pid);
    if (to_arrive == 0)
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
    return bar.acquire(arg);
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

template <size_t chan_len>
class mutex
{
public:
  const char *const name;

private:
  mutable chan_t<chan_len> lock_chan;
  mutable bool busy = false;

public:
  mutex() : name{""}, lock_chan{}, busy{false} {}

  mutex(const char *const name) : name{name}, lock_chan{}, busy{false} {}

  mutex(const mutex &) = delete;
  mutex &operator=(const mutex &) = delete;

  mutex(mutex &&) = default;
  mutex &operator=(mutex &&) = default;

  void lock() const
  {
    while (busy)
      sched::wait(lock_chan);

    busy = true;
  }

  void unlock() const
  {
    assert(busy, S_RESET);
    busy = false;
    sched::notify(lock_chan);
  }
};

template <typename _mutex>
  requires requires(_mutex &_m) {
    _m.lock();
    _m.unlock();
  }
class lock_guard
{
private:
  _mutex &_m;

public:
  lock_guard(_mutex &_m) : _m{_m} { _m.lock(); }

  lock_guard(_mutex &_m, const char *str) : _m{_m}
  {
    debug<INFO>("lock at %s\r\n", str);
    _m.lock();
  }

  ~lock_guard() { _m.unlock(); }

  lock_guard(const lock_guard &) = delete;
  lock_guard &operator=(const lock_guard &) = delete;

  lock_guard(lock_guard &&) = delete;
  lock_guard &operator=(lock_guard &&) = delete;
};

