#pragma once

#include "core/memory.h"
#include "core/signal.h"
#include "core/types.h"
#include "utility/debug.h"

namespace
{
const char *states[] = {
    "[empty]",
    GREEN "[runnable]" DEFAULT,
    BLUE "[asleep]" DEFAULT,
    YELLOW "[running]" DEFAULT,
};

const char *priority_levels[] = {
    "idle", "low", "medium", "high", "urgent", "highest",
};
} // namespace

struct proc_def_t;
struct barrier_t;

struct proc_t {
  string name = {};
  priority_t priority = EMPTY;

  byte_t *stk_ptr = nullptr;
  byte_t *stack = nullptr;
  size_t stk_sz = 0;

  const proc_def_t *def = nullptr;
  void *param = nullptr;
  chunk_t used_hd = {};

  bool term_req = false;
  barrier_t *bar = nullptr;
  signals_t signals = {};

  void reset()
  {
    priority = EMPTY;

    term_req = false;
    bar = nullptr;

    signals = {};
  }

  void trace(pid_t pid, uint32_t total_ticks, bool curr)
  {
    int state = 0;
    if (curr)
      state = 3;
    else if (priority > 0)
      state = 1;
    else if (priority < 0)
      state = 2;

    auto prio = abs(priority);
    int prio_lev;
    if (prio < LOW1)
      prio_lev = 0;
    else if (prio < MID1)
      prio_lev = 1;
    else if (prio < HIGH1)
      prio_lev = 2;
    else if (prio < URGENT1)
      prio_lev = 3;
    else if (prio < HIGHEST)
      prio_lev = 4;
    else
      prio_lev = 5;

    rational_t perc{(int32_t)def->ticks * 100, total_ticks};
    debug<INFO>("  |  %d. " BLUE "%s" DEFAULT " : (%s), %s, %f%\r\n", pid,
                name.str, priority_levels[prio_lev], states[state], perc);
    debug<TRACE>("  |    stack: %p, sz: %d, stk_ptr: %p\r\n", stack, stk_sz,
                 stk_ptr);
    debug<TRACE>("  |    heap usage:\r\n");
    for (auto ptr = &used_hd; ptr->next != nullptr; ptr = ptr->next) {
      debug<TRACE>("  |      %x: %d\r\n", ptr->next, ptr->next->sz);
    }
  }
};

template <uint8_t _N_PROCS>
class procs_t
{
  proc_t procs[_N_PROCS] = {};

public:
  inline proc_t *alloc()
  {
    uint8_t pid = 0;
    while (pid < _N_PROCS && procs[pid].priority != EMPTY)
      pid++;

    if (pid == _N_PROCS)
      return nullptr;

    return &procs[pid];
  }

  inline void dealloc(proc_t *proc) { proc->reset(); }

  proc_t *max_priority()
  {
    proc_t *max_proc = nullptr;
    priority_t max_priority = 0;

    for (pid_t pid = 0; pid < _N_PROCS; pid++) {
      auto &proc = procs[pid];

      if (max_priority >= proc.priority)
        continue;

      max_proc = &proc;
      max_priority = proc.priority;
    }

    return max_proc;
  }

  void trace(pid_t curr, uint32_t total_ticks)
  {
    for (auto pid = 0; pid < _N_PROCS; pid++) {
      auto &proc = procs[pid];
      if (proc.priority != EMPTY)
        proc.trace(pid, total_ticks, pid == curr);
    }
  }

  inline pid_t pid(proc_t *proc) const
  {
    assert(proc != nullptr, S_RESET);
    return proc - procs;
  }

  inline proc_t *operator[](pid_t pid)
  {
    assert(pid < _N_PROCS, S_RESET);
    return &procs[pid];
  }
};
