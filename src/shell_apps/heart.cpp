#include "display.h"
#include "lib.h"
#include "serial.h"
#include "shell.h"
#include "signal.h"
#include "types.h"

namespace shell
{

namespace
{

const image_t big_heart = IMAGE(0, 1, 0, 1, 0,  //
                                1, 1, 1, 1, 1,  //
                                1, 1, 1, 1, 1,  //
                                0, 1, 1, 1, 0,  //
                                0, 0, 1, 0, 0); //

const image_t small_heart = IMAGE(0, 0, 0, 0, 0,  //
                                  0, 1, 0, 1, 0,  //
                                  0, 1, 1, 1, 0,  //
                                  0, 0, 1, 0, 0,  //
                                  0, 0, 0, 0, 0); //

static bool exit = false;

void heart(void *param)
{
  swap_handler(SIGTERM, []() { exit = true; });

  auto buf = (args_buffer_t *)param;
  auto str = buf->args;

  auto n = (*str == '\0') ? 100 : atoi(str);

  while (!exit && n-- > 0) {
    display::show(big_heart);
    sched::sleep(500);
    display::show(small_heart);
    sched::sleep(100);
    display::show(big_heart);
    sched::sleep(100);
    display::show(small_heart);
    sched::sleep(100);

    display::reset();
  }

  exit = false;
  display::reset();
}

} // namespace

proc_def_t heart_cmd = {"heart", LOW3, 128, heart};

} // namespace shell
