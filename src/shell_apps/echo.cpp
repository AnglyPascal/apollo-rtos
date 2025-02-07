#include "display.h"
#include "memory.h"
#include "serial.h"
#include "shell.h"
#include "types.h"

namespace shell
{

namespace
{

void echo(void *param)
{
  auto buf = (args_buffer_t *)param;
  printf(">> %s\r\n", buf->args);
}

void clear(void *param)
{
  serial::clear_screen();
  display::reset();
}

} // namespace

proc_def_t echo_cmd = {"echo", MID4, 128, echo};
proc_def_t clear_cmd = {"clear", MID4, 24, clear};

} // namespace shell
