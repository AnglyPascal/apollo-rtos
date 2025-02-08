#include "drivers/accel.h"
#include "core/fs.h"
#include "core/shell.h"
#include "core/signal.h"
#include "core/types.h"
#include "drivers/display.h"
#include "drivers/serial.h"
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

constexpr size_t len = (pg_sz - circular_buffer_header_sz) / sizeof(accel_t);
using buffer = circular_buffer<accel_t, len>;

static_assert(sizeof(buffer) % sizeof(uint32_t) == 0);

volatile bool exit = false;

bool listener(char c)
{
  if (c == CTRL('q')) {
    exit = true;
    return true;
  }
  return false;
}

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
  swap_handler(SIGTERM, []() { exit = true; });

  auto file = fs::open(accel::fn, sizeof(buffer), O_CREATE);
  auto val = (buffer *)*file;

  bool run_bg = ((shell::args_buffer_t *)param)->run_bg;
  serial::listener_guard guard{listener, !run_bg};

  constexpr int threshold = 10;
  auto is_zero = [](int p) { return (-threshold < p) && (p < threshold); };

  while (!exit) {
    if (!run_bg)
      serial::clear_screen();

    if (!val->empty()) {
      auto [x, y, z] = val->back();

      auto dx = is_zero(x) ? 1 : (x < 0 ? 0 : 2);
      auto dy = is_zero(y) ? 1 : (y < 0 ? 2 : 0);
      display::show(dirs[dy][dx]);

      if (!run_bg)
        printf("x: %d, y: %d, z: %d\r\n", x, y, z);
    }

    sched::sleep(50);
  }

  exit = false;
  display::reset();
}

} // namespace

proc_def_t accel_cmd = {"accel", HIGH1, 128, accel};

} // namespace shell
