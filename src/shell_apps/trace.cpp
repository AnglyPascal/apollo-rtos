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
  if (c == 'q') {
    exit = true;
    return true;
  }
  return false;
}

void *trace(void *param)
{
  serial::register_listener(listener);

  while (!exit) {
    serial::clear_screen();
    sched::trace();
    waitlist::trace();
    sched::sleep(1000);
  }
  serial::clear_screen();
  exit = false;

  serial::unregister_listener();
  return param;
}

} // namespace

cmd_t trace_cmd = {"trace", 4, 128, trace};

} // namespace shell
