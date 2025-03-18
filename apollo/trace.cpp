#include "core/boot.h"
#include "core/memory.h"
#include "core/signal.h"
#include "core/types.h"
#include "core/waitlist.h"
#include "drivers/serial.h"
#include "fs/fs.h"
#include "utility/args.h"
#include "utility/profile.h"

namespace
{
__always_inline__
inline void do_trace()
{
  kprintf("\r\nboot level: %s\r\n", boot::lev_str());
  sched::trace();
  waitlist::trace();
  profile::trace();
  fs::trace();
  heap::trace();
}

APP(trace, MID4, 128, param)
{
  auto &buf = *(args_t *)param;
  auto args = buf.str;

  bool run_once = false;
  if (*args++ == '-' && *args++ == 'r')
    run_once = true;

  sigterm_listener_t<__COUNTER__> sig_guard{!run_once};

  if (run_once) {
    sched::decr_priority(LOW3);
    while (!curr_proc::term_req()) {
      clear_screen();
      do_trace();
      sched::sleep(1000);
    }
    clear_screen();
  } else {
    do_trace();
  }
}
} // namespace

