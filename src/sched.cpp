#include "sched.h"
#include "allocator.h"
#include "debug.h"
#include "hardware.h"
#include "irq.h"
#include "lib.h"
#include "memory.h"
#include "serial.h"
#include "timer.h"
#include "types.h"
#include "waitlist.h"

#include <cassert>

namespace sched
{

// stack layout for interrupt frames
struct stack_t {
  // saved manually
  uint32_t r8;  // 5
  uint32_t r9;  // 6
  uint32_t r10; // 7
  uint32_t r11; // 8

  uint32_t r4;      // 1
  uint32_t r5;      // 2
  uint32_t r6;      // 3
  uint32_t r7;      // 4
  uint32_t lr_intr; // 0

  // saved by hardware
  uint32_t r0;  // 9
  uint32_t r1;  // 10
  uint32_t r2;  // 11
  uint32_t r3;  // 12
  uint32_t r12; // 13
  uint32_t lr;  // 14
  uint32_t pc;  // 15
  uint32_t psr; // 16
};

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

  stack_t *stk_ptr;
  uint8_t *stack;
  size_t stk_sz;
};

constexpr uint8_t N_PROCS = 16;
proc_t procs[N_PROCS] = {{state_t::EMPTY, nullptr, 0, nullptr, 0}};

constexpr priority_t IDLE_PRIORITY = 1;

// Thumb bit is set
constexpr uint32_t init_psr = 0x01000000;

// magic return address for exceptions
constexpr uint32_t lr_intr_magic = 0xfffffff9;
// FIXME: shouldn't it be 0xfffffffd ???!!!

struct {
  proc_t *hi_proc = nullptr;
  proc_t *curr_proc = nullptr;
  time_t last_checked;
} cpu;

namespace
{
allocator pool{alloc_stack, 16};
} // namespace

void end_proc(void *param)
{
  debug<TRACE>("\t\t\tending %s\r\n", cpu.curr_proc->name.str);
  cpu.curr_proc->state = state_t::EMPTY;
  heap::free(param);
  pool.dealloc(cpu.curr_proc->stack);
  decr_priority(0);
}

proc_t *reg_proc(string name, priority_t priority, uint32_t stk_sz,
                 runnable_t func, void *param)
{
  uint8_t pid = 0;
  while (pid < N_PROCS && procs[pid].state != state_t::EMPTY)
    pid++;

  if (pid == N_PROCS) {
    // FIXME: panic
    return nullptr;
  }

  auto &proc = procs[pid];

  proc.name = name;
  proc.state = state_t::RUNNABLE;

  proc.stk_sz = stk_sz;
  proc.stack = pool.alloc(stk_sz);

  // FIXME: set a minimum stack size

  // TODO: maybe abstract out the fact that effective stack size is stk_sz -
  // sizeof(stack_t)
  proc.stk_ptr = (stack_t *)(proc.stack + stk_sz - sizeof(stack_t));

  debug<TRACE>("reg_proc: %s, %d, %x, %x\r\n", name.str, priority, proc.stack,
               proc.stk_ptr);

  if (proc.stack == nullptr) {
    // FIXME: panic
    return nullptr;
  }

  // setup initial stack frame
  auto stk_ptr = proc.stk_ptr;
  *stk_ptr = {0};
  stk_ptr->psr = init_psr;
  stk_ptr->pc = (uint32_t)func;
  stk_ptr->lr = (uint32_t)end_proc;
  stk_ptr->r0 = (uint32_t)param;
  stk_ptr->lr_intr = lr_intr_magic;

  incr_priority(&proc, priority);

  return &proc;
}

void change_proc()
{
  cpu.last_checked = timer::now();
  if (cpu.hi_proc != cpu.curr_proc)
    reschedule();
}

extern "C" void *cxt_switch(void *stk_ptr)
{
  if (cpu.hi_proc->state != state_t::RUNNABLE) {
    // FIXME: panic!!
    return stk_ptr;
  }

  debug<TRACE>("\t\t\t\t\"%s\" -> \"%s\"\r\n", cpu.curr_proc->name.str,
               cpu.hi_proc->name.str);

  if (cpu.curr_proc->state == state_t::RUNNING)
    cpu.curr_proc->state = state_t::RUNNABLE;
  cpu.hi_proc->state = state_t::RUNNING;

  cpu.curr_proc->stk_ptr = (stack_t *)stk_ptr;
  cpu.curr_proc = cpu.hi_proc;
  return cpu.hi_proc->stk_ptr;
}

void *invoke(void *curr_stk, time_t millis)
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
  debug<TRACE>("\t\t\tincr prio, %s: %d -> %d\r\n", proc->name.str,
               proc->priority, priority);

  proc->priority = priority;
  if (cpu.hi_proc == nullptr || cpu.hi_proc->priority < priority) {
    cpu.hi_proc = proc;
  }
}

void decr_priority(priority_t priority)
{
  debug<TRACE>("\t\t\tdecr prio, %s: %d -> %d\r\n", cpu.curr_proc->name.str,
               cpu.curr_proc->priority, priority);

  if (cpu.curr_proc->priority < priority) {
    // FIXME: panic
    return;
  }

  cpu.curr_proc->priority = priority;

  proc_t *max_proc = cpu.curr_proc;
  priority_t max_priority = max_proc->priority;

  for (pid_t pid = 0; pid < N_PROCS; pid++) {
    if (procs[pid].state != state_t::RUNNABLE ||
        max_priority >= procs[pid].priority)
      continue;

    max_proc = &procs[pid];
    max_priority = procs[pid].priority;
  }

  if (max_proc == cpu.curr_proc) {
    return;
  }

  cpu.hi_proc = max_proc;
  change_proc();
}

void print_procs()
{
  for (int i = 0; i < N_PROCS; i++) {
    if (procs[i].state == state_t::RUNNABLE ||
        procs[i].state == state_t::RUNNING) {
      printf("%s: %d, ", procs[i].name.str, procs[i].priority);
    }
  }
  printf("\r\n");
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
extern "C" void __run(void *(*task)(void *), stack_t **stk_ptr);

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
  disable_irq(TIMER1_IRQ);
  auto proc = (proc_t *)ptr;

  proc->state = state_t::RUNNABLE;
  incr_priority(proc, -proc->priority);
  enable_irq(TIMER1_IRQ);
}

proc_t *default_alarm_param;

void sleep(time_t period)
{
  disable_irq(TIMER1_IRQ);

  auto proc = cpu.curr_proc;
  cpu.curr_proc->state = state_t::ASLEEP;
  waitlist::reg(period, default_alarm, cpu.curr_proc);
  enable_irq(TIMER1_IRQ);

  decr_priority(-proc->priority);
}

void proc_trace(proc_t *proc)
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

void trace()
{
  printf("sched:\r\n");
  for (auto proc = procs; proc < procs + N_PROCS; proc++) {
    if (proc->state != state_t::EMPTY)
      proc_trace(proc);
  }
}

} // namespace sched
