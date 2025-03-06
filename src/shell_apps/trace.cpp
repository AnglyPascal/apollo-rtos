#include "core/memory.h"
#include "core/shell.h"
#include "core/signal.h"
#include "core/types.h"
#include "core/waitlist.h"
#include "drivers/serial.h"
#include "fs/fs.h"

namespace shell
{

namespace
{
__always_inline__
inline void do_trace()
{
  sched::trace();
  waitlist::trace();
  fs::trace();
  heap::trace();
}

void trace(void *param)
{
  auto &buf = *(args_t *)param;
  auto args = buf.str;

  bool run_bg = false;
  if (*args++ == '-' && *args++ == 'r')
    run_bg = true;

  sigterm_listener_t<__COUNTER__> sig_guard{run_bg};

  if (run_bg)
    sched::decr_priority(LOW3);

  if (run_bg) {
    while (!curr_proc::term_req()) {
      serial::clear_screen();
      do_trace();
      sched::sleep(1000);
    }
    serial::clear_screen();
  } else {
    do_trace();
  }
}

} // namespace

proc_def_t trace_cmd = {"trace", MID4, 128, trace};

} // namespace shell
