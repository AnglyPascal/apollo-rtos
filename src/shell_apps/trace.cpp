#include "core/fs.h"
#include "core/memory.h"
#include "core/shell.h"
#include "core/types.h"
#include "core/waitlist.h"
#include "drivers/serial.h"

namespace shell
{

namespace
{

volatile bool exit = false;

bool listener(char c)
{
  if (c == CTRL('q')) {
    exit = true;
    return true;
  }
  return false;
}

__always_inline__
inline void do_trace()
{
  sched::trace();
  waitlist::trace();
  /* fs::trace(); */
  heap::trace();
}

void trace(void *param)
{
  auto &buf = *(args_t *)param;
  auto args = buf.str;

  bool resume = false;
  if (*args++ == '-' && *args++ == 'r')
    resume = true;

  serial::listener_guard guard{listener, resume};

  if (resume)
    sched::decr_priority(LOW3);

  if (resume) {
    while (!exit) {
      serial::clear_screen();
      do_trace();
      sched::sleep(1000);
    }
    serial::clear_screen();
  } else {
    do_trace();
  }
  exit = false;
}

} // namespace

proc_def_t trace_cmd = {"trace", MID4, 128, trace};

} // namespace shell
