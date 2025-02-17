#pragma once

#include "core/sched.h"
#include "core/types.h"

template <size_t chan_len = N_PROCS_WAIT>
class mutex
{
/* private: */

public:
  string name;
  chan_t<chan_len> lock_chan;
  bool busy = false;

  void lock()
  {
    while (busy) {
      sched::wait(lock_chan);
    }
    busy = true;
  }

  void unlock()
  {
    assert(busy);
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
