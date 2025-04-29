#include "core/boot.h"
#include "core/memory.h"
#include "core/signal.h"
#include "core/types.h"
#include "core/waitlist.h"
#include "drivers/serial.h"
#include "fs/fs.h"
#include "utility/args.h"
#include "utility/profile.h"

namespace sched
{
void trace(bool stk_info);
}

namespace waitlist
{
void trace();
}

namespace
{
__always_inline__ inline void do_trace(bool stk_info = false)
{
  kprintf("boot level: " BOLD YELLOW "%s" DEFAULT "\r\n", boot::lev_str());
  sched::trace(stk_info);
  waitlist::trace();
  fs::trace();
  heap::trace();
}

APP(htop, MID4, 128, param)
{
  auto args = (args_t *)param;
  bool run_once = args->get_option() == 'r';

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

