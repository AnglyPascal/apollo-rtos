#include "core/signal.h"
#include "core/types.h"
#include "drivers/display.h"
#include "utility/args.h"

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

APP(heart, LOW3, 128, param)
{
  auto args = (args_t *)param;
  auto n = args->get_uint().value_or(100);

  sigterm_listener_t<__COUNTER__> guard{false};

  while (!curr_proc::term_req() && n-- > 0) {
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
