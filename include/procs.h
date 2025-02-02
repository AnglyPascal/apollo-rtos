#pragma once

#include "debug.h"
#include "irq.h"
#include "memory.h"
#include "serial.h"
#include "types.h"

namespace
{
const char *states[] = {
    "[empty]",
    "[runnable]",
    "[asleep]",
    "[running]",
};
}

struct proc_t {
  string name = {};
  priority_t priority = 0;

  byte_t *stk_ptr = nullptr;
  byte_t *stack = nullptr;
  size_t stk_sz = 0;

  chunk_t used_hd = {};
};

template <uint8_t N_PROCS>
class procs_t
{
  proc_t procs[N_PROCS] = {{}};

public:
  inline proc_t *alloc()
  {
    intr_guard guard;

    uint8_t pid = 0;
    while (pid < N_PROCS && procs[pid].priority != 0)
      pid++;

    if (pid == N_PROCS) {
      return nullptr;
    }

    return &procs[pid];
  }

  inline void dealloc(proc_t *proc)
  {
    intr_guard guard;
    proc->priority = 0;
  }

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
      if (proc->priority != 0)
        proc_trace(proc);
    }
  }

  inline void proc_trace(proc_t *proc)
  {
    auto pid = proc - procs;

    int state = 0;
    if (proc->priority > 0)
      state = 1;
    else if (proc->priority < 0)
      state = 2;

    printf("\t%d. %s : %d, %s\r\n", pid, proc->name.str, proc->priority,
           states[state]);
    printf("\t\tstack: %p, sz: %d, stk_ptr: %p\r\n", proc->stack, proc->stk_sz,
           proc->stk_ptr);
    for (auto ptr = &proc->used_hd; ptr->next != nullptr; ptr = ptr->next) {
      printf("\t\t%x: %d\r\n", ptr, ptr->sz);
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
