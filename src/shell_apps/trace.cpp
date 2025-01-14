#include "memory.h"
#include "serial.h"
#include "shell.h"
#include "types.h"
#include "waitlist.h"

namespace shell
{

namespace
{

void *trace(void *param)
{
  printf("\r\n");
  sched::trace();
  /* heap::trace(); */
  waitlist::trace();
  return param;
}

} // namespace

cmd_t trace_cmd = {"trace", 4, 128, trace};

} // namespace shell
