#pragma once

#include "core/memory.h"
#include "core/recover.h"
#include "core/signal.h"
#include "core/types.h"
#include "drivers/serial.h"
#include "utility/debug.h"

namespace
{
const char *states[] = {
    "[empty]",
    "[runnable]",
    "[asleep]",
    "[running]",
};

const char *priority_levels[] = {
    "idle", "low", "medium", "high", "urgent", "highest",
};
} // namespace

struct proc_def_t;

struct proc_t {
  string name = {};
  priority_t priority = EMPTY;

  byte_t *stk_ptr = nullptr;
  byte_t *stack = nullptr;
  size_t stk_sz = 0;

  proc_def_t *def;
  void *param;
  chunk_t used_hd = {};

  rec_id_t rec_id = null_rec_id;
  signals_t signals = {};

  void trace(pid_t pid)
  {
    int state = 0;
    if (priority > 0)
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

    debug<INFO>("  |  %d. %s : (%s), %s\r\n", pid, name.str,
                priority_levels[prio_lev], states[state]);
    debug<TRACE>("  |    stack: %p, sz: %d, stk_ptr: %p\r\n", stack, stk_sz,
                 stk_ptr);
    debug<TRACE>("  |    heap usage:\r\n");
    for (auto ptr = &used_hd; ptr->next != nullptr; ptr = ptr->next) {
      debug<TRACE>("  |      %x: %d\r\n", ptr->next, ptr->next->sz);
    }
  }
};

template <uint8_t N_PROCS>
class procs_t
{
  proc_t procs[N_PROCS] = {};

public:
  inline proc_t *alloc()
  {
    uint8_t pid = 0;
    while (pid < N_PROCS && procs[pid].priority != 0)
      pid++;

    if (pid == N_PROCS) {
      return nullptr;
    }

    return &procs[pid];
  }

  inline void dealloc(proc_t *proc) { proc->priority = 0; }

  proc_t *max_priority()
  {
    proc_t *max_proc = nullptr;
    priority_t max_priority = 0;

    for (pid_t pid = 0; pid < N_PROCS; pid++) {
      if (max_priority >= procs[pid].priority)
        continue;

      max_proc = &procs[pid];
      max_priority = procs[pid].priority;
    }

    return max_proc;
  }

  void trace()
  {
    for (auto proc = procs; proc < procs + N_PROCS; proc++) {
      if (proc->priority != EMPTY) {
        auto _pid = pid(proc);
        proc->trace(_pid);
      }
    }
  }

  inline pid_t pid(proc_t *proc) const
  {
    assert(proc != nullptr);
    return proc - procs;
  }

  inline proc_t *operator[](pid_t pid)
  {
    assert(pid >= 0 && pid < N_PROCS, "pid: %d\r\n", pid);
    return &procs[pid];
  }
};
