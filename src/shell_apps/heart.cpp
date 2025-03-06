#include "core/shell.h"
#include "core/signal.h"
#include "core/types.h"
#include "drivers/display.h"

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

void heart(void *param)
{
  sigterm_handler_t<__COUNTER__> handler;

  auto buf = (args_t *)param;
  auto str = buf->str;

  auto n = (*str == '\0') ? 10 : atoi(str);

  while (handler.run() && n-- > 0) {
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

  display::reset();
}

} // namespace

proc_def_t heart_cmd = {"heart", LOW3, 128, heart};

} // namespace shell
