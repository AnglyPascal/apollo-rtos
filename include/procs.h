#pragma once

#include "debug.h"
#include "irq.h"
#include "serial.h"
#include "types.h"

enum class state_t : uint8_t {
  EMPTY,
  RUNNABLE,
  RUNNING,
  ASLEEP,
};

namespace
{
const char *states[] = {
    "[empty]",
    "[runnable]",
    "[running]",
    "[asleep]",
};
}

struct proc_t {
  state_t state;
  string name;
  priority_t priority;

  uint8_t *stk_ptr;
  uint8_t *stack;
  size_t stk_sz;
};

template <uint8_t N_PROCS>
class procs_t
{
  proc_t procs[N_PROCS] = {{state_t::EMPTY, nullptr, 0, nullptr, nullptr, 0}};

public:
  inline proc_t *alloc()
  {
    intr_guard guard;

    uint8_t pid = 0;
    while (pid < N_PROCS && procs[pid].state != state_t::EMPTY)
      pid++;

    if (pid == N_PROCS) {
      return nullptr;
    }

    procs[pid].state = state_t::RUNNABLE;
    return &procs[pid];
  }

  inline void dealloc(proc_t *proc)
  {
    intr_guard guard;
    proc->state = state_t::EMPTY;
  }

  proc_t *max_priority()
  {
    proc_t *max_proc = nullptr;
    priority_t max_priority = 0;

    for (pid_t pid = 0; pid < N_PROCS; pid++) {
      if (procs[pid].state != state_t::RUNNABLE ||
          max_priority >= procs[pid].priority)
        continue;

      max_proc = &procs[pid];
      max_priority = procs[pid].priority;
    }

    return max_proc;
  }

  void trace()
  {
    for (auto proc = procs; proc < procs + N_PROCS; proc++) {
      if (proc->state != state_t::EMPTY)
        proc_trace(proc);
    }
  }

  inline void proc_trace(proc_t *proc)
  {
    auto pid = proc - procs;
    auto state = states[static_cast<size_t>(proc->state)];
    printf("\t%d. %s : %d, %s\r\n", pid, proc->name.str, proc->priority, state);
    printf("\t\tstack: %p, sz: %d, stk_ptr: %p\r\n", proc->stack, proc->stk_sz,
           proc->stk_ptr);
  }

  inline pid_t pid(proc_t *proc) const
  {
    return proc - procs;
  }
};
