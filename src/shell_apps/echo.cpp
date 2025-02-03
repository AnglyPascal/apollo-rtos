#include "memory.h"
#include "display.h"
#include "serial.h"
#include "shell.h"
#include "types.h"

namespace shell
{

namespace
{

void *echo(void *param)
{
  auto buf = (buffer *)param;
  printf(">> %s\r\n", buf->args);
  return param;
}

void *clear(void *param)
{
  serial::clear_screen();
  display::reset();
  return param;
}

} // namespace

proc_def_t echo_cmd = {"echo", 4, 128, echo, nullptr};
proc_def_t clear_cmd = {"clear", 4, 24, clear, nullptr};

} // namespace shell
