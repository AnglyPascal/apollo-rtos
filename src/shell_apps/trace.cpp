#include "memory.h"
#include "serial.h"
#include "shell.h"
#include "types.h"

namespace shell
{

namespace
{

void trace(void *)
{
  printf("\r\n");
  sched::trace();
  heap::trace();
}

} // namespace

cmd_t trace_cmd = {"trace", 3, 68, trace};

} // namespace shell
