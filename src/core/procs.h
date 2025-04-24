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
  priority_lev_t lev;
  uint8_t weight = 0;

  priority_t(priority_lev_t lev) : lev{lev}, weight{0} {}
  priority_t(int lev) : lev{(priority_lev_t)lev}, weight{0} {}

  priority_t &operator=(int32_t lev)
  {
    this->lev = (priority_lev_t)lev;
    this->weight = 0;
    return *this;
  }

  friend auto operator<=>(const priority_t &lhs, const priority_t &rhs)
  {
    if (lhs.lev != rhs.lev)
      return lhs.lev <=> rhs.lev;
    return (rhs.weight) <=> lhs.weight;
  }

  friend auto operator==(const priority_t &lhs, int32_t rhs)
  {
    return lhs.lev == rhs;
  }

  friend auto operator!=(const priority_t &lhs, int32_t rhs)
  {
    return lhs.lev != rhs;
  }

  friend auto operator==(int32_t lhs, const priority_t &rhs)
  {
    return rhs == lhs;
  }

  friend auto operator!=(int32_t lhs, const priority_t &rhs)
  {
    return rhs != lhs;
  }

  friend auto operator<=>(const priority_t &lhs, int32_t rhs)
  {
    return lhs.lev <=> (priority_lev_t)rhs;
  }

  friend auto operator<=>(int32_t lhs, const priority_t &rhs)
  {
    return (priority_lev_t)lhs <=> rhs.lev;
  }
};

struct proc_def_t;
struct barrier_t;

struct proc_t {
  string name = {};
  priority_t priority{EMPTY};

  byte_t *stk_ptr = nullptr;
  byte_t *stack = nullptr;
  size_t stk_sz = 0;

  const proc_def_t *def = nullptr;
  void *param = nullptr;
  chunk_list_t used_list = {};

  fn_t out_fn = stdout;
  bool term_req = false;
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

  void trace(pid_t pid, uint32_t total_ticks, bool curr)
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

    rational_t perc{(int32_t)def->ticks, total_ticks};
    debug<INFO>("  |  "                       //
                BOLD YELLOW "%d" DEFAULT ". " //
                BOLD CYAN "%s" DEFAULT " : (%s), %s, %f%",
                pid, name.str, priority_levels[prio_lev], states[state], perc);
    debug<INFO>("\t"                           //
                "stk: " BLUE "%p" DEFAULT ", " //
                "sz: " BLUE "%d" DEFAULT ", "  //
                "stk_ptr: " BLUE "%p" DEFAULT "\r\n",
                stack, stk_sz, stk_ptr);

    debug<TRACE>("  |    heap usage:\r\n");
    for (auto it = used_list.begin(); it != used_list.end(); ++it)
      debug<TRACE>("  |      %x: %d\r\n", &*it, it->sz);
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
    priority_t max_priority{0};

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

  inline void init_lists()
  {
    for (auto &proc : procs)
      proc.used_list.init();
  }
};
