#pragma once

#include "core/sched.h"
#include "core/types.h"
#include "utility/debug.h"

template <size_t chan_len>
class mutex
{
public:
  const char *name;

private:
  chan_t<chan_len> lock_chan;
  bool busy = false;

public:
  mutex() : name{""}, lock_chan{}, busy{false} {}

  mutex(const char *name) : name{name}, lock_chan{}, busy{false} {}

  void lock()
  {
    while (busy) {
      sched::wait(lock_chan);
    }
    busy = true;
  }

  void unlock()
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
};


