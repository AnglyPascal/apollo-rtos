#pragma once

#include "debug.h"
#include "serial.h"
#include "types.h"

using pid_t = uint8_t;

enum class state_t : uint8_t {
  EMPTY,
  RUNNABLE,
  RUNNING,
  ASLEEP,
};

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

    const char *state;
    if (proc->state == state_t::ASLEEP)
      state = "asleep";
    else if (proc->state == state_t::RUNNABLE)
      state = "runnable";
    else if (proc->state == state_t::RUNNING)
      state = "running";
    else
      state = "empty";

    printf("\t%d. %s : %s\r\n", pid, proc->name.str, state);
    printf("\t\tstack: %x, sz: %d, stk_ptr: %x\r\n", proc->stack, proc->stk_sz,
           proc->stk_ptr);
  }
};
