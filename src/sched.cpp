#include "sched.h"
#include "allocator.h"
#include "debug.h"
#include "irq.h"
#include "proc_stack.h"
#include "procs.h"
#include "serial.h"
#include "timer.h"
#include "types.h"
#include "waitlist.h"

namespace sched
{

constexpr size_t N_PROCS = 16;
procs_t<N_PROCS> procs;

constexpr priority_t IDLE_PRIORITY = 1;

struct {
  proc_t *hi_proc = nullptr;
  proc_t *curr_proc = nullptr;
  time_t last_checked;
} cpu;

namespace
{
stack_t stack;
} // namespace

void end_proc(void *param)
{
  debug<TRACE>("\t\t\tending %s\r\n", cpu.curr_proc->name.str);
  heap::free(param);
  decr_priority(0);
}

proc_t *reg_proc(string name, priority_t priority, uint32_t stk_sz,
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

void change_proc()
{
  cpu.last_checked = timer::now();
  if (cpu.hi_proc != cpu.curr_proc)
    reschedule();
}

extern "C" uint8_t *cxt_switch(uint8_t *stk_ptr)
{
  if (cpu.hi_proc->state != state_t::RUNNABLE) {
    debug<FATAL>("!! cpu.hi_proc is not runnable\r\n");
    return stk_ptr;
  }

  debug<TRACE>("\t\t\t\t\"%s\" -> \"%s\"\r\n", cpu.curr_proc->name.str,
               cpu.hi_proc->name.str);

  if (cpu.curr_proc->priority == 0) {
    stack.release(cpu.curr_proc);
    procs.dealloc(cpu.curr_proc);
  } else {
    cpu.curr_proc->state = state_t::RUNNABLE;
    cpu.curr_proc->stk_ptr = stk_ptr;
  }

  assert(cpu.hi_proc->stack <= cpu.hi_proc->stk_ptr);

  cpu.hi_proc->state = state_t::RUNNING;
  cpu.curr_proc = cpu.hi_proc;
  return cpu.hi_proc->stk_ptr;
}

uint8_t *invoke(uint8_t *curr_stk, time_t millis)
{
  if ((millis & ((invoke_interval >> 1) - 1)) ||
      millis <= cpu.last_checked + invoke_interval)
    return curr_stk;

  cpu.last_checked = timer::now();
  if (cpu.hi_proc == cpu.curr_proc)
    return curr_stk;

  return cxt_switch(curr_stk);
}

void incr_priority(proc_t *proc, priority_t priority)
{
  assert(cpu.curr_proc->priority < priority);
  debug<TRACE>("\t\t\tincr prio, %s: %d -> %d\r\n", proc->name.str,
               proc->priority, priority);

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

  cpu.curr_proc->priority = priority;

  proc_t *max_proc = procs.max_priority();
  if (max_proc == cpu.curr_proc) {
    return;
  }

  cpu.hi_proc = max_proc;
  change_proc();
}

void *idle_task(void *param)
{
  change_proc();

  bool go = false;
  while (1) {
    bool prev = go;
    go = (timer::now() & 1023) == 0;

    if (!prev && go) {
      led_dot();
    } else if (prev && !go) {
      led_off();
    }

    change_proc();
  }
  return param;
}

/* enter thread mode with specified stack (see mpx.s) */
extern "C" void __run(void *(*task)(void *), uint8_t **stk_ptr);

void setup_procs(void);

/* assign idle_task to the main process, sets up all the other processes, then
 * enter the idle_task in thread mode */
void init()
{
  auto idle_proc = reg_proc("idle_proc", IDLE_PRIORITY, 0, idle_task, nullptr);
  idle_proc->state = state_t::RUNNING;

  cpu.curr_proc = idle_proc;
  setup_procs();
  cpu.last_checked = timer::now();

  __run(idle_task, &idle_proc->stk_ptr);
}

void default_alarm(void *ptr)
{
  auto proc = (proc_t *)ptr;
  proc->state = state_t::RUNNABLE;
  incr_priority(proc, -proc->priority);
}

void sleep(time_t period)
{
  disable_irq(TIMER1_IRQ);

  auto proc = cpu.curr_proc;
  proc->state = state_t::ASLEEP;
  waitlist::reg(period, default_alarm, proc);

  enable_irq(TIMER1_IRQ);

  decr_priority(-proc->priority);
}

void trace()
{
  printf("sched:\r\n");
  procs.trace();
}

} // namespace sched
