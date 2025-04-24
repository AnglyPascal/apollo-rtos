#include "core/sched.h"

#include "core/irq.h"
#include "core/recover.h"
#include "core/test.h"
#include "core/types.h"
#include "core/waitlist.h"
#include "drivers/display.h"
#include "drivers/i2c.h"
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "utility/args.h"
#include "utility/debug.h"
#include "utility/format.h"
#include "utility/profile.h"

#include "procs.h"
#include "stack_allocator.h"

namespace heap
{
void cleanup();
}

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

// manuall initialize all the linked-lists
void init_lists()
{
  procs.init_lists();
  return stack_allocator.init_list();
}

volatile time_t last_checked = 0;

void exit()
{
  auto proc = cpu.curr_proc;

  if (proc->bar != nullptr)
    proc->bar->release();

  kmem::kfree(proc->param);
  heap::cleanup();

  intr_disable();
  decr_priority(0);
}

pid_t _reg_proc_impl(const proc_def_t *def, void *param)
{
  auto [name, priority, stk_sz, func, ticks] = *def;
  assert(priority > 0, H_RESET);

  intr_guard guard;

  auto proc = procs.alloc();
  auto pid = procs.pid(proc);
  assert(proc != nullptr, S_RESET);

  proc->name = name;
  proc->param = param;
  proc->def = def;

  proc->out_fn = stdout;
  proc->term_req = false;
  new (&proc->signals) signals_t{};

  stack_allocator.acquire(proc, stk_sz, func, param, exit);
  incr_priority(pid, priority);

  return pid;
}

void set_out_fn(pid_t pid, fn_t out_fn)
{
  auto proc = procs[pid];
  proc->out_fn = out_fn;
}

void notify_exit(pid_t pid, barrier_t *bar)
{
  auto proc = procs[pid];
  assert(proc->bar == nullptr, S_RESET);
  proc->bar = bar;
}

bool needs_swap() { return cpu.hi_proc != cpu.curr_proc; }

__noinline__ void change_proc()
{
  intr_enable();
  last_checked = timer::now();
  if (needs_swap())
    reschedule();
}

__extern_C__ void *cxt_switch(void *stk_ptr)
{
  PROFILE_THIS(2);

  assert(cpu.hi_proc->priority > 0, H_RESET);
  assert(cpu.hi_proc->stack <= cpu.hi_proc->stk_ptr, H_RESET);

  intr_guard guard;

  auto proc = (proc_t *)cpu.curr_proc;
  if (proc->priority == 0) {
    stack_allocator.release(proc);
    procs.dealloc(proc);
  } else {
    proc->stk_ptr = (byte_t *)stk_ptr;
  }

  cpu.curr_proc = cpu.hi_proc;
  cpu.curr_proc->priority.weight++;
  return cpu.hi_proc->stk_ptr;
}

void incr_priority(pid_t pid, int32_t priority)
{
  auto proc = procs[pid];
  assert(proc->priority < priority, S_RESET);

  intr_guard guard;

  proc->priority.lev = (priority_lev_t)priority;
  if (cpu.hi_proc == nullptr || cpu.hi_proc->priority < priority) {
    cpu.hi_proc = proc;
  }
}

void decr_priority(int32_t priority)
{
  assert(cpu.curr_proc->priority > priority, S_RESET);

  cpu.curr_proc->priority.lev = (priority_lev_t)priority;
  cpu.hi_proc = procs.max_priority();

  change_proc();
}

void yield()
{
  cpu.hi_proc = procs.max_priority();
  change_proc();
}

namespace
{
static pid_t IDLE_PID = 0;

PROC(idle, IDLE, 8, param)
{
  intr_enable();
  while (true) {
    change_proc(); // NOTE: comment to test invoker
  }
}
} // namespace

SEC_ADDR(services);
SEC_ADDR(startups);

void __weak__ setup_services(void)
{
  for (SEC_ITER(services, proc_def_t, proc))
    reg_proc(proc, nullptr);
}

void __weak__ setup_procs(void)
{
  for (SEC_ITER(startups, proc_def_t, proc))
    reg_proc(proc, nullptr);
}

/* enter idle_task with specified stack (see mpx.s) */
__extern_C__ void __run(runnable_t task, byte_t **stk_ptr);

/* assign idle_task to the main process, sets up all the other processes, then
 * enter the idle_task in thread mode */
void init()
{
  intr_disable();

  IDLE_PID = REG_PROC(idle, nullptr);
  cpu.curr_proc = procs[IDLE_PID];

  setup_services();
  recover::setup();

  timer::init();
  last_checked = timer::now();

  cpu.set_up = true;
  cpu.curr_proc->stk_ptr = cpu.curr_proc->stack + cpu.curr_proc->stk_sz - 16;
  __run(PROC_FUNC(idle), &cpu.curr_proc->stk_ptr);
}

void trace(bool stk_info)
{
  debug<INFO>(BOLD "curr_proc: " YELLOW "%s" DEFAULT "\r\n",
              cpu.curr_proc->name.str);
  procs.trace(curr_proc::pid(), stk_info);
}

void default_alarm(void *ptr)
{
  auto proc = (proc_t *)ptr;
  assert(proc->priority < 0, S_RESET);
  incr_priority(procs.pid(proc), -proc->priority.lev);
}

void sleep(time_t period)
{
  auto proc = cpu.curr_proc;
  assert(proc->priority > 0, S_RESET);

  if (period > 0)
    waitlist::reg(proc->name, period, default_alarm, (void *)proc);

  decr_priority(-proc->priority.lev);
}

void wakeup(pid_t pid) { incr_priority(pid, -procs[pid]->priority.lev); }

void assert_stack()
{
  if (cpu.curr_proc == nullptr)
    return;

  auto stack = (void *)cpu.curr_proc->stack;
  auto stack_end = (uint8_t *)stack + cpu.curr_proc->stk_sz;
  auto curr_stk = (void *)get_msp();

  assert(stack <= curr_stk && curr_stk <= stack_end, H_RESET,
         "\r\n%s, %x, %x, %x\r\n", cpu.curr_proc->name.str, stack, curr_stk,
         stack_end);
}

void tick() { cpu.curr_proc->def->tick(); }

} // namespace sched

using namespace sched;

namespace curr_proc
{
pid_t pid() { return procs.pid(cpu.curr_proc); }
string name() { return cpu.curr_proc->name; }
const proc_def_t *def() { return cpu.curr_proc->def; }
bool term_req() { return cpu.curr_proc->term_req; }
bool set_up() { return cpu.set_up; }

chunk_list_t &used_list() { return cpu.curr_proc->used_list; }
fn_t out_fn() { return !set_up() ? stdout : cpu.curr_proc->out_fn; }
} // namespace curr_proc

///////////////
/// SIGNALS ///
///////////////

template <>
void default_handler<SIGTERM>(void)
{
  cpu.curr_proc->term_req = true;
}

template <>
void default_handler<SIGKILL>(void)
{
  exit();
}

__extern_C__ void handle_signals(void)
{
  cpu.curr_proc->signals.handle_signals();
}

signal_handler_t swap_handler(signal_t sig, signal_handler_t new_handler)
{
  return cpu.curr_proc->signals.swap(sig, new_handler);
}

void send_signal(pid_t pid, signal_t sig)
{
  assert(pid <= N_PROCS, H_RESET);
  procs[pid]->signals.send(sig);
}

void trigger_term(const char *file, uint32_t line)
{
  if (curr_proc::set_up()) {
    debug<ERROR>("terminating " YELLOW "%s" RED " from " YELLOW
                 "%s:%d\r\n" DEFAULT,
                 curr_proc::name().str, file, line);
    return send_signal(curr_proc::pid(), SIGKILL);
  }
  return trigger_reset();
}

namespace
{
APP(pkill, HIGHEST, 128, param)
{
  auto buf = (args_t *)param;
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
      if (procs[pid]->name == string{args})
        break;
    }
  }

  if (pid >= N_PROCS || procs[pid]->priority == EMPTY)
    return debug<ERROR>("Process not found\r\n");

  if (pid == IDLE_PID)
    return debug<FATAL>("Cannot kill idle_proc\r\n");

  send_signal(pid, kill ? SIGKILL : SIGTERM);
  sched::wait(pid);

  debug<INFO>("killed [" BLUE "%d" DEFAULT "]\r\n", pid);
}
} // namespace
