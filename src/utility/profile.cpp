#include "utility/profile.h"

#include "core/sched.h"
#include "core/types.h"
#include "drivers/timer.h"
#include "utility/debug.h"

namespace
{
__always_inline__ inline void func_trace()
{
  debug<INFO>(BOLD "funcs:\r\n" DEFAULT);
  for (auto [func_name, ticks] : profile::tbl) {
    if (func_name == nullptr || ticks == 0)
      return;

    debug<INFO>("  |  " BLUE "%s" DEFAULT ": " YELLOW "%f%" DEFAULT "\r\n",
                func_name, rational_t{(int32_t)ticks, timer::total_ticks()});
  }
}

SEC_ADDR(services);
SEC_ADDR(startups);
SEC_ADDR(apps);

__always_inline__ inline void proc_trace()
{
  debug<INFO>(BOLD "procs:\r\n" DEFAULT);

  uint32_t total_ticks = timer::total_ticks();
  uint32_t sum_ticks = 0;

  auto prof = [&](auto proc) {
    if (proc->ticks == 0)
      return;
    sum_ticks += proc->ticks;

    rational_t perc{(int32_t)proc->ticks, total_ticks};
    debug<INFO>("  |  " CYAN "%s" DEFAULT ": "
                "(" YELLOW "%f%" DEFAULT ")\r\n",
                proc->name.str, perc);
  };

  for (SEC_ITER(services, proc_def_t, proc))
    prof(proc);
  for (SEC_ITER(startups, proc_def_t, proc))
    prof(proc);
  for (SEC_ITER(apps, proc_def_t, proc))
    prof(proc);

  auto idle_ticks = total_ticks - sum_ticks;
  rational_t perc{(int32_t)idle_ticks, total_ticks};
  debug<INFO>("  |  " BOLD CYAN "idle" DEFAULT ": "
              "(" YELLOW "%f%" DEFAULT ")\r\n",
              perc);
}

APP(prof, MID4, 128, param)
{
  proc_trace();
  func_trace();
  debug<INFO>("\r\n");
}
} // namespace
