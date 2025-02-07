#include "sched.h"
#include "circular_buffer.h"
#include "debug.h"
#include "irq.h"
#include "lib.h"
#include "proc_stack.h"
#include "procs.h"
#include "recover.h"
#include "serial.h"
#include "shell.h"
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

template <bool kill>
void exit()
{
  debug<TRACE>("| ending %s, kill: %d\r\n", cpu.curr_proc->name.str, kill);

  kmem::kfree(cpu.curr_proc->param);
  heap::cleanup(&cpu.curr_proc->used_hd);

  if constexpr (kill) {
    recovery::set_rec_lev(rec_lev_t::NONE);
  }

  intr_disable();
  decr_priority(0);
}

proc_t *reg_proc(proc_def_t *proc_def)
{
  auto [name, priority, stk_sz, func, param] = *proc_def;
  return reg_proc(name, priority, stk_sz, func, param);
}

size_t curr_proc_rec_entry_id() { return cpu.curr_proc->rec_entry_id; }

proc_t *reg_proc(string name, priority_t priority, size_t stk_sz,
                 runnable_t func, void *param)
{
  assert(priority > 0, "%s\r\n", name.str);

  intr_guard guard;

  auto proc = procs.alloc();
  assert(proc != nullptr, "%s\r\n", name.str);

  proc->name = name;
  proc->param = param;

  proc->rec_entry_id = recovery::get_rec_entry_id();
  stack.acquire(proc, stk_sz, func, param, exit<true>);

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
  assert(cpu.hi_proc->stack <= cpu.hi_proc->stk_ptr);

  debug<TRACE>("\t\t\t\t\"%s\" -> \"%s\"\r\n", cpu.curr_proc->name.str,
               cpu.hi_proc->name.str);

  intr_guard guard;

  if (cpu.curr_proc->priority == 0) {
    stack.release((proc_t *)cpu.curr_proc);
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

static pid_t IDLE_PID = 0;

void idle_task(void *)
{
  while (true) {
    change_proc(); // NOTE: comment to test invoker
  }
}

/* enter idle_task with specified stack (see mpx.s) */
__extern_C__
void __run(runnable_t task, byte_t **stk_ptr);

void setup_procs(void);

/* assign idle_task to the main process, sets up all the other processes, then
 * enter the idle_task in thread mode */
void init()
{
  auto idle_proc = reg_proc("idle_proc", IDLE_PRIORITY, 0, idle_task, nullptr);
  IDLE_PID = procs.pid(idle_proc);
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
  debug<INFO>("  curr_proc: %s\r\n", cpu.curr_proc->name.str);
  procs.trace();
}

pid_t curr_pid() { return procs.pid(cpu.curr_proc); }

// TODO: write doc for these, or rename
chunk_t *curr_proc_used_hd() { return &cpu.curr_proc->used_hd; }

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

void wait(chan_t *chan)
{
  chan->pid = curr_pid();
  decr_priority(-cpu.curr_proc->priority);
}

void notify(chan_t *chan)
{
  assert(*chan->event);
  auto proc = procs[chan->pid];
  incr_priority(proc, -proc->priority);
}

string curr_proc_name() { return cpu.curr_proc->name; }

void give_up_param() { cpu.curr_proc->param = nullptr; }

} // namespace sched

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
  auto buf = (shell::buffer *)param;
  auto args = buf->args;

  bool kill = args[0] == '-' && args[1] == '9';
  if (kill) {
    args += 2;
    while (*args == ' ')
      args++;
  }

  pid_t pid;
  if (*args <= '9' && *args >= '0') {
    pid = atoi(buf->args);
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

proc_def_t pkill_cmd = {"pkill", _max<priority_t>, 32, pkill, nullptr};
} // namespace shell
