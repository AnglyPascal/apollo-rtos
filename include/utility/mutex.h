#pragma once

#include "core/sched.h"
#include "core/types.h"
#include "utility/debug.h"

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

  mutex(const char *name) : name{name}, lock_chan{}, busy{false} {}

  mutex(const mutex &) = delete;
  mutex &operator=(const mutex &) = delete;

  mutex(mutex &&) = default;
  mutex &operator=(mutex &&) = default;

  void lock() const
  {
    while (busy) {
      sched::wait(lock_chan);
    }
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
  ~lock_guard() { _m.unlock(); }

  lock_guard(const lock_guard &) = delete;
  lock_guard &operator=(const lock_guard &) = delete;

  lock_guard(lock_guard &&) = delete;
  lock_guard &operator=(lock_guard &&) = delete;
};

