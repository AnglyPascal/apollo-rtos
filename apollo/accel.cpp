#include "drivers/accel.h"
#include "core/recover.h"
#include "core/sched.h"
#include "core/signal.h"
#include "core/types.h"
#include "drivers/display.h"
#include "drivers/serial.h"
#include "fs/fs.h"
#include "utility/args.h"
#include "utility/circular_buffer.h"

#include <utility>

namespace
{
inline constexpr fn_t fn = 5;

constexpr size_t len = 8;
using buffer = circular_buffer<accel::data_t, len>;

const image_t dirs[3][3] = {
    {
        IMAGE(1, 0, 0, 0, 0,  //
              0, 1, 0, 0, 0,  //
              0, 0, 1, 0, 0,  //
              0, 0, 0, 0, 0,  //
              0, 0, 0, 0, 0), //
        IMAGE(0, 0, 1, 0, 0,  //
              0, 0, 1, 0, 0,  //
              0, 0, 1, 0, 0,  //
              0, 0, 0, 0, 0,  //
              0, 0, 0, 0, 0), //
        IMAGE(0, 0, 0, 0, 1,  //
              0, 0, 0, 1, 0,  //
              0, 0, 1, 0, 0,  //
              0, 0, 0, 0, 0,  //
              0, 0, 0, 0, 0), //
    },
    {
        IMAGE(0, 0, 0, 0, 0,  //
              0, 0, 0, 0, 0,  //
              1, 1, 1, 0, 0,  //
              0, 0, 0, 0, 0,  //
              0, 0, 0, 0, 0), //
        IMAGE(0, 0, 0, 0, 0,  //
              0, 0, 0, 0, 0,  //
              0, 0, 1, 0, 0,  //
              0, 0, 0, 0, 0,  //
              0, 0, 0, 0, 0), //
        IMAGE(0, 0, 0, 0, 0,  //
              0, 0, 0, 0, 0,  //
              0, 0, 1, 1, 1,  //
              0, 0, 0, 0, 0,  //
              0, 0, 0, 0, 0), //
    },
    {
        IMAGE(0, 0, 0, 0, 0,  //
              0, 0, 0, 0, 0,  //
              0, 0, 1, 0, 0,  //
              0, 1, 0, 0, 0,  //
              1, 0, 0, 0, 0), //
        IMAGE(0, 0, 0, 0, 0,  //
              0, 0, 0, 0, 0,  //
              0, 0, 1, 0, 0,  //
              0, 0, 1, 0, 0,  //
              0, 0, 1, 0, 0), //
        IMAGE(0, 0, 0, 0, 0,  //
              0, 0, 0, 0, 0,  //
              0, 0, 1, 0, 0,  //
              0, 0, 0, 1, 0,  //
              0, 0, 0, 0, 1), //
    },
};

APP(accel, HIGH1, 128, param)
{
  auto file = fram::open(fn, O_SHARED);
  auto val = file.mmap<const buffer>();

  bool run_bg = ((args_t *)param)->run_bg;
  sigterm_listener_t<__COUNTER__> guard{run_bg};

  constexpr int threshold = 10;
  auto is_zero = [](int p) { return (-threshold < p) && (p < threshold); };


  uint8_t n = 0;
  while (!curr_proc::term_req()) {
    if (!val->empty()) {
      auto [x, y, z] = val->back();

      auto dx = is_zero(x) ? 1 : (x < 0 ? 0 : 2);
      auto dy = is_zero(y) ? 1 : (y < 0 ? 2 : 0);
      display::show(dirs[dy][dx]);

      if (n++ % 16 == 0 && !run_bg) {
        clear_screen();
        printf("x: %d, y: %d, z: %d\r\n", x, y, z);
      }
    }

    sched::sleep(50);
  }

  clear_screen();
  display::reset();
}

int n __recover_section__ = 100;

STARTUP_PROC(accel_bg, HIGH3, 256, param)
{
  recover::guard_proc guard{POWER_OFF};

  auto file = fram::open<buffer>(fn, O_WRITE | O_CREATE | O_SHARED);
  auto buf = file.mmap<buffer>();

  accel::init();

  uint8_t n = MAX<uint8_t>;
  while (true) {
    buf->enqueue(accel::read());
    if (n-- == 0)
      file.store();
    sched::sleep(200);
  }
  file.store();
}
} // namespace
