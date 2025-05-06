#include "core/sched.h"
#include "core/types.h"
#include "drivers/display.h"
#include "drivers/serial.h"
#include "utility/args.h"
#include "utility/iostream.h"

namespace
{
APP(echo, MID4, 128, param)
{
  auto buf = (args_t *)param;
  printf("%s", buf->str);
  fprintf(stdout, "\r\n");
}

APP(clear, MID4, 24, param)
{
  clear_screen();
  display::reset();
}
} // namespace

