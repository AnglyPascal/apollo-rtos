#include "core/sched.h"

#include "core/irq.h"
#include "core/recover.h"
#include "core/shell.h"
#include "core/types.h"
#include "core/waitlist.h"
#include "drivers/i2c.h"
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "utility/debug.h"
#include "utility/lib.h"

#include "procs.h"
#include "stack_allocator.h"

namespace sched
{

namespace
{
procs_t<N_PROCS> procs;
stack_allocator_t stack_allocator;

volatile struct {
  proc_t *hi_proc = nullptr;
  proc_t *curr_proc = nullptr;
  bool set_up = false;
} cpu = {};
} // namespace

volatile time_t last_checked = 0;

template <bool kill>
void exit()
{
  debug<TRACE>("| ending %s, kill: %d\r\n", cpu.curr_proc->name.str, kill);

  kmem::kfree(cpu.curr_proc->param);
  heap::cleanup(&cpu.curr_proc->used_hd);

  intr_disable();
  decr_priority(0);
}

proc_t *reg_proc(proc_def_t *proc_def, void *param)
{
  auto [name, priority, stk_sz, func] = *proc_def;

  assert(priority > 0, "%x, %s\r\n", priority, name.str);

  intr_guard guard;

  auto proc = procs.alloc();
  assert(proc != nullptr, "%s\r\n", name.str);

  proc->name = name;
  proc->param = param;
  proc->def = proc_def;

  stack_allocator.acquire(proc, stk_sz, func, param, exit<true>);

  incr_priority(proc, priority);

  debug<TRACE>("reg_proc: %s, %d, %x, %x\r\n", name.str, priority, proc->stack,
               proc->stk_ptr);
  return proc;
}

bool needs_swap() { return cpu.hi_proc != cpu.curr_proc; }

__noinline__
void change_proc()
{
  intr_enable();
  last_checked = timer::now();
  if (needs_swap())
    reschedule();
}

__extern_C__
void *cxt_switch(void *stk_ptr)
{
  assert(cpu.hi_proc->priority > 0);
  assert_dump(cpu.hi_proc->stack <= cpu.hi_proc->stk_ptr,
         "hi_proc: %x, stack: %x, stk_ptr:  %x\r\n", cpu.hi_proc,
         cpu.hi_proc->stack, cpu.hi_proc->stk_ptr);

  debug<TRACE>("\t\t\t\t\"%s\" -> \"%s\"\r\n", cpu.curr_proc->name.str,
               cpu.hi_proc->name.str);

  intr_guard guard;

  if (cpu.curr_proc->priority == 0) {
    stack_allocator.release((proc_t *)cpu.curr_proc);
    procs.dealloc((proc_t *)cpu.curr_proc);
  } else {
    cpu.curr_proc->stk_ptr = (byte_t *)stk_ptr;
  }

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
  assert(cpu.curr_proc->priority > priority,
         "\r\nproc: %s, previous: %d, new: %d\r\n", cpu.curr_proc->name.str,
         cpu.curr_proc->priority, priority);
  debug<TRACE>("\t\t\tdecr prio, %s: %d -> %d\r\n", cpu.curr_proc->name.str,
               cpu.curr_proc->priority, priority);

  cpu.curr_proc->priority = priority;
  cpu.hi_proc = procs.max_priority();

  change_proc();
}

namespace
{

static pid_t IDLE_PID = 0;

void idle_task(void *)
{
  intr_enable();
  while (true) {
    change_proc(); // NOTE: comment to test invoker
  }
}

proc_def_t idle_proc_def = {"idle_proc", IDLE, 8, idle_task};
} // namespace

/* enter idle_task with specified stack (see mpx.s) */
__extern_C__
void __run(runnable_t task, byte_t **stk_ptr);

/* assign idle_task to the main process, sets up all the other processes, then
 * enter the idle_task in thread mode */
void init()
{
  intr_disable();

  auto idle_proc = reg_proc(&idle_proc_def, nullptr);
  IDLE_PID = procs.pid(idle_proc);
  cpu.curr_proc = idle_proc;

  recover::init();
  shell::init();

  timer::init();
  last_checked = timer::now();

  cpu.set_up = true;
  idle_proc->stk_ptr = idle_proc->stack + idle_proc->stk_sz - 16;
  __run(idle_task, &idle_proc->stk_ptr);
}

void trace()
{
  debug<INFO>("  curr_proc: %s\r\n", cpu.curr_proc->name.str);
  procs.trace();
}

void default_alarm(void *ptr)
{
  auto proc = (proc_t *)ptr;
  assert(proc->priority < 0, "alarm: \"%s\" not asleep\r\n", proc->name.str);
  incr_priority(proc, -proc->priority);
}

void sleep(time_t period)
{
  auto proc = cpu.curr_proc;
  assert(proc->priority > 0, "sleep1\r\n");
  waitlist::reg(proc->name, period, default_alarm, (void *)proc);
  decr_priority(-proc->priority);
}

void sleep() { decr_priority(-cpu.curr_proc->priority); }

void wakeup(pid_t pid)
{
  auto proc = procs[pid];
  incr_priority(proc, -proc->priority);
}

void transfer_param(void *param)
{
  assert(param == cpu.curr_proc->param);
  cpu.curr_proc->param = nullptr;
}

void assert_stack()
{
  if (cpu.curr_proc == nullptr)
    return;

  auto stack = (void *)cpu.curr_proc->stack;
  auto stack_end = (uint8_t *)stack + cpu.curr_proc->stk_sz;
  auto curr_stk = (void *)get_msp();
  assert(stack <= curr_stk && curr_stk <= stack_end,
         "stack: %x, curr_stk: %x, stack_end: %x\r\n", stack, curr_stk,
         stack_end);
}

} // namespace sched

namespace curr_proc
{
using namespace sched;
pid_t pid() { return procs.pid(cpu.curr_proc); }
string name() { return cpu.curr_proc->name; }
chunk_t *used_hd() { return &cpu.curr_proc->used_hd; }
proc_def_t *def() { return cpu.curr_proc->def; }

bool set_up() { return cpu.set_up; }
} // namespace curr_proc

///////////////
/// SIGNALS ///
///////////////

template <>
void default_handler<SIGTERM>(void)
{
  sched::exit<false>();
}

template <>
void default_handler<SIGKILL>(void)
{
  sched::exit<true>();
}

__extern_C__
void handle_signals(void)
{
  // FIXME: kprintf causes a sleep/wake up issue, investigate later
  /* printf("sig\r\n"); */

  sched::cpu.curr_proc->signals.handle_signals();
}

signal_handler_t swap_handler(signal_t sig, signal_handler_t new_handler)
{
  return sched::cpu.curr_proc->signals.swap(sig, new_handler);
}

void send_signal(pid_t pid, signal_t sig)
{
  if (pid < 0 || pid > N_PROCS) {
    debug<ERROR>("pid %u out of range\r\n", pid);
    return;
  }

  sched::procs[pid]->signals.send(sig);
}

namespace shell
{
void pkill(void *param)
{
  auto buf = (shell::args_t *)param;
  auto args = buf->str;

  bool kill = args[0] == '-' && args[1] == '9';
  if (kill) {
    args += 2;
    while (*args == ' ')
      args++;
  }

  pid_t pid;
  if (*args <= '9' && *args >= '0') {
    pid = atoi(buf->str);
  } else {
    for (pid = 0; pid < N_PROCS; pid++) {
      if (sched::procs[pid]->name == string{args})
        break;
    }
  }

  if (pid >= N_PROCS) {
    debug<ERROR>("Process not found\r\n");
    return;
  }

  if (pid == sched::IDLE_PID) {
    debug<FATAL>("Cannot kill idle_proc\r\n");
    return;
  }

  send_signal(pid, kill ? SIGKILL : SIGTERM);
}

proc_def_t pkill_cmd = {"pkill", HIGHEST, 128, pkill};
} // namespace shell
