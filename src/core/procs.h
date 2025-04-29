#pragma once

#include "core/signal.h"
#include "core/types.h"
#include "utility/allocator.h"
#include "utility/debug.h"

#include <compare>

namespace
{
const char *states[] = {
    RED "[empty]" DEFAULT,
    GREEN "[runnable]" DEFAULT,
    BLUE "[asleep]" DEFAULT,
    YELLOW "[running]" DEFAULT,
};

const char *priority_levels[] = {
    LIGHT_BLACK "idle" DEFAULT, GREEN "low" DEFAULT,  YELLOW "medium" DEFAULT,
    YELLOW "high" DEFAULT,      RED "urgent" DEFAULT, RED "highest" DEFAULT,
};
} // namespace

struct priority_t {
  priority_lev_t lev = EMPTY;
  uint8_t weight = 0;

  priority_t() {}
  priority_t(priority_lev_t lev) : lev{lev}, weight{0} {}
  priority_t(int lev) : lev{(priority_lev_t)lev}, weight{0} {}

  priority_t &operator=(priority_lev_t lev)
  {
    this->lev = lev;
    this->weight = 0;
    return *this;
  }

  friend auto operator<=>(const priority_t &lhs, const priority_t &rhs)
  {
    if (lhs.lev != rhs.lev)
      return lhs.lev <=> rhs.lev;
    return (rhs.weight) <=> lhs.weight;
  }

  bool asleep() const { return lev < 0; }
  bool awake() const { return lev > 0; }

  static_assert(EMPTY == 0);
  bool empty() const { return lev == EMPTY; }
};

struct proc_def_t;
class barrier_t;

struct proc_t {
  priority_t priority{EMPTY};

  fn_t out_fn = stdout;
  bool term_req = false;

  byte_t *stk_ptr = nullptr;
  byte_t *stack = nullptr;
  size_t stk_sz = 0;

  const proc_def_t *def = nullptr;
  void *param = nullptr;
  chunk_list_t used_list = {};

  barrier_t *bar = nullptr;
  signals_t signals = {};

  void reset()
  {
    priority = EMPTY;

    out_fn = stdout;
    term_req = false;
    bar = nullptr;

    signals.reset();
  }

  const char *name() const { return def == nullptr ? nullptr : def->name; }

  void trace(pid_t pid, bool curr, bool stk_info) const
  {
    int state = 0;
    if (curr)
      state = 3;
    else if (priority > 0)
      state = 1;
    else if (priority < 0)
      state = 2;

    auto prio = abs(priority.lev);
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

    debug<INFO>("  |  "                       //
                BOLD YELLOW "%d" DEFAULT ". " //
                BOLD CYAN "%s" DEFAULT " : (%s), %s",
                pid, name(), priority_levels[prio_lev], states[state]);

    (stk_info) ? debug<INFO>("\t"                           //
                             "stk: " BLUE "%p" DEFAULT ", " //
                             "sz: " BLUE "%d" DEFAULT ", "  //
                             "stk_ptr: " BLUE "%p" DEFAULT "\r\n",
                             stack, stk_sz, stk_ptr)
               : debug<INFO>("\r\n");

    debug<TRACE>("  |    heap usage:\r\n");
    for (auto it = used_list.begin(); it != used_list.end(); ++it)
      debug<TRACE>("  |      %x: %d\r\n", &*it, it->sz);
  }
};

class procs_t
{
  proc_t procs[N_PROCS] = {};

public:
  inline proc_t *alloc()
  {
    uint8_t pid = 0;
    while (pid < N_PROCS && !procs[pid].priority.empty())
      pid++;

    if (pid == N_PROCS)
      return nullptr;

    return &procs[pid];
  }

  inline void dealloc(proc_t *proc) { proc->reset(); }

  proc_t *max_priority()
  {
    intr_guard guard;

    proc_t *max_proc = nullptr;
    priority_t max_priority{0};

    for (pid_t pid = 0; pid < N_PROCS; pid++) {
      auto &proc = procs[pid];

      if (max_priority >= proc.priority)
        continue;

      max_proc = &proc;
      max_priority = proc.priority;
    }

    return max_proc;
  }

  void trace(pid_t curr, bool stk_info) const
  {
    for (auto pid = 0; pid < N_PROCS; pid++) {
      auto &proc = procs[pid];
      if (!proc.priority.empty())
        proc.trace(pid, pid == curr, stk_info);
    }
  }

  inline pid_t pid(proc_t *proc) const
  {
    assert(proc != nullptr, S_RESET);
    return proc - procs;
  }

  inline proc_t *operator[](pid_t pid)
  {
    assert(pid < N_PROCS, S_RESET);
    return &procs[pid];
  }

  inline void init_lists()
  {
    for (auto &proc : procs)
      proc.used_list.init();
  }
};
