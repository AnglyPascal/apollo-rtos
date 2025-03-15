#include "drivers/accel.h"
#include "core/shell.h"
#include "core/signal.h"
#include "core/types.h"
#include "drivers/display.h"
#include "drivers/serial.h"
#include "fs/fs.h"
#include "utility/circular_buffer.h"

#include <utility>

namespace shell
{

namespace
{

struct alignas(uint32_t) accel_t {
  int x;
  int y;
  int z;
};

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

void accel(void *param)
{
  auto file = fram::open<accel::buffer>(accel::fn, O_SHARED);
  auto val = file.mmap<const accel::buffer>();

  bool run_bg = ((shell::args_t *)param)->run_bg;
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
        serial::clear_screen();
        printf("x: %d, y: %d, z: %d\r\n", x, y, z);
      }
    }

    sched::sleep(50);
  }

  serial::clear_screen();
  display::reset();
}

} // namespace

proc_def_t accel_cmd = {"accel", HIGH1, 128, accel};

} // namespace shell
