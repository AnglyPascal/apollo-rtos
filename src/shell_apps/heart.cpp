#include "display.h"
#include "serial.h"
#include "shell.h"
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

void *heart(void *param)
{
  while (1) {
    display::show(big_heart);
    sched::sleep(100);
    display::show(small_heart);
    sched::sleep(10);
    display::show(big_heart);
    sched::sleep(10);
    display::show(small_heart);
  }
  return param;
}

} // namespace

cmd_t heart_cmd = {"heart", 3, 68, heart};

} // namespace shell
