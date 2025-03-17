#include "core/shell.h"
#include "core/types.h"
#include "drivers/display.h"
#include "drivers/serial.h"

namespace
{
APP(echo, MID4, 128, param)
{
  auto buf = (shell::args_t *)param;
  printf(">> %s\r\n", buf->str);
}

APP(clear, MID4, 24, param)
{
  serial::clear_screen();
  display::reset();
}
} // namespace

