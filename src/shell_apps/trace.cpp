#include "fs.h"
#include "memory.h"
#include "serial.h"
#include "shell.h"
#include "types.h"
#include "waitlist.h"

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

void *trace(void *param)
{
  serial::listener_guard guard{listener};

  while (!exit) {
    serial::clear_screen();
    sched::trace();
    waitlist::trace();
    fs::trace();
    heap::trace();
    sched::sleep(3000);
  }
  serial::clear_screen();
  exit = false;

  return param;
}

} // namespace

proc_def_t trace_cmd = {"trace", 2, 128, trace, nullptr};

} // namespace shell
