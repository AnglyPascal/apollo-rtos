#include "sched.h"
#include "debug.h"
#include "irq.h"
#include "proc_stack.h"
#include "procs.h"
#include "recover.h"
#include "serial.h"
#include "timer.h"
#include "types.h"
#include "waitlist.h"

namespace sched
{

namespace
{
constexpr priority_t IDLE_PRIORITY = 1;

procs_t<N_PROCS> procs;
stack_t stack;

volatile struct {
  proc_t *hi_proc = nullptr;
  proc_t *curr_proc = nullptr;
} cpu;
} // namespace

volatile time_t last_checked = 0;

void end_proc(void *param)
{
  debug<TRACE>("\t\t\tending %s\r\n", cpu.curr_proc->name.str);
  heap::free(param);
  recovery::set_rec_lev(rec_lev_t::NONE);
  decr_priority(0);
}

proc_t *reg_proc(proc_def_t *proc_def)
{
  auto [name, priority, stk_sz, func, param] = *proc_def;
  return reg_proc(name, priority, stk_sz, func, param);
}

proc_t *reg_proc(string name, priority_t priority, size_t stk_sz,
                 runnable_t func, void *param)
{
  assert(priority > 0);

  auto proc = procs.alloc();
  assert(proc != nullptr);

  proc->name = name;
  proc->state = state_t::RUNNABLE;

  stack.acquire(proc, stk_sz, func, param, end_proc);
  incr_priority(proc, priority);

  debug<TRACE>("reg_proc: %s, %d, %x, %x\r\n", name.str, priority, proc->stack,
               proc->stk_ptr);
  return proc;
}

bool needs_swap() { return cpu.hi_proc != cpu.curr_proc; }

void change_proc()
{
  last_checked = timer::now();
  if (needs_swap())
    reschedule();
}

__extern_C__
void *cxt_switch(void *stk_ptr)
{
  assert(cpu.hi_proc->state == state_t::RUNNABLE);
  assert(cpu.hi_proc->stack <= cpu.hi_proc->stk_ptr);

  debug<TRACE>("\t\t\t\t\"%s\" -> \"%s\"\r\n", cpu.curr_proc->name.str,
               cpu.hi_proc->name.str);

  intr_guard guard;

  if (cpu.curr_proc->priority == 0) {
    stack.release((proc_t *)cpu.curr_proc);
    procs.dealloc((proc_t *)cpu.curr_proc);
  } else {
    if (cpu.curr_proc->state != state_t::ASLEEP)
      cpu.curr_proc->state = state_t::RUNNABLE;
    cpu.curr_proc->stk_ptr = (byte_t *)stk_ptr;
  }

  cpu.hi_proc->state = state_t::RUNNING;
  cpu.curr_proc = cpu.hi_proc;
  return cpu.hi_proc->stk_ptr;
}

void incr_priority(proc_t *proc, priority_t priority)
{
  assert(proc->priority < priority, "\r\nproc: %s, previous: %d, new: %d\r\n",
         proc->name.str, proc->priority, priority);
  debug<TRACE>("\t\t\tincr prio, %s: %d -> %d\r\n", proc->name.str,
               proc->priority, priority);

  intr_guard guard;

  proc->priority = priority;
  if (cpu.hi_proc == nullptr || cpu.hi_proc->priority < priority) {
    cpu.hi_proc = proc;
  }
}

void decr_priority(priority_t priority)
{
  assert(cpu.curr_proc->priority >= priority);
  debug<TRACE>("\t\t\tdecr prio, %s: %d -> %d\r\n", cpu.curr_proc->name.str,
               cpu.curr_proc->priority, priority);

  {
    intr_guard guard;

    cpu.curr_proc->priority = priority;
    proc_t *max_proc = procs.max_priority();
    cpu.hi_proc = max_proc;
  }

  change_proc();
}

__attribute__((optimize("O1"))) // O2 doesn't work
void *
idle_task(void *)
{
  while (true) {
    change_proc(); // NOTE: comment to test invoker
  }
  return nullptr;
}

/* enter idle_task with specified stack (see mpx.s) */
__extern_C__
void __run(void *(*task)(void *), byte_t **stk_ptr);

void setup_procs(void);

/* assign idle_task to the main process, sets up all the other processes, then
 * enter the idle_task in thread mode */
void init()
{
  auto idle_proc = reg_proc("idle_proc", IDLE_PRIORITY, 0, idle_task, nullptr);
  idle_proc->state = state_t::RUNNING;

  cpu.curr_proc = idle_proc;

  if (is_first_boot()) {
    setup_procs();
  } else {
    recover();
  }

  last_checked = timer::now();

  __run(idle_task, &idle_proc->stk_ptr);
}

void trace()
{
  printf("sched:\r\n");
  procs.trace();
}

pid_t curr_pid() { return procs.pid(cpu.curr_proc); }

chunk_t *curr_proc_used_hd() { return &cpu.curr_proc->used_hd; }

void default_alarm(void *ptr)
{
  auto proc = (proc_t *)ptr;
  assert(proc->priority < 0 && proc->state == state_t::ASLEEP,
         "alarm: \"%s\" not asleep\r\n", proc->name.str);
  proc->state = state_t::RUNNABLE;
  incr_priority(proc, -proc->priority);
}

void sleep(time_t period)
{
  auto proc = cpu.curr_proc;
  assert(proc->state != state_t::ASLEEP, "sleep2\r\n");

  {
    intr_guard guard;
    proc->state = state_t::ASLEEP;
    waitlist::reg(proc->name, period, default_alarm, (void *)proc);
  }

  decr_priority(-proc->priority);
}

void sleep()
{
  auto proc = cpu.curr_proc;
  assert(proc->state != state_t::ASLEEP, "sleep2\r\n");

  proc->state = state_t::ASLEEP;
  decr_priority(-proc->priority);
}

void wakeup(pid_t pid)
{
  auto proc = procs[pid];

  assert(proc->priority < 0 && proc->state == state_t::ASLEEP,
         "wakeup: \"%s\" not asleep\r\n", proc->name.str);
  proc->state = state_t::RUNNABLE;
  incr_priority(proc, -proc->priority);
}

string curr_proc_name() { return cpu.curr_proc->name; }

} // namespace sched
