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

constexpr size_t len = (pg_sz - circular_buffer_header_sz) / sizeof(accel_t);
using buffer = circular_buffer<accel_t, len>;

static_assert(sizeof(buffer) % sizeof(uint32_t) == 0);

// FIXME: definitely can be made into a more programmatic solution
pid_t curr_accel_pid = null_pid;
bool listener(char c)
{
  if (c == CTRL('q')) {
    send_signal(curr_accel_pid, SIGTERM);
    return true;
  }
  return false;
}

void accel(void *param)
{
  curr_accel_pid = curr_proc::pid();
  sigterm_handler_t<__COUNTER__> handler;

  auto file = flash::open(accel::fn, sizeof(buffer), O_CREATE | O_SHARED);
  auto val = file.mmap<buffer>();

  bool run_bg = ((shell::args_t *)param)->run_bg;
  serial::listener_guard guard{listener, !run_bg};

  constexpr int threshold = 10;
  auto is_zero = [](int p) { return (-threshold < p) && (p < threshold); };

  while (handler.run()) {
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

  display::reset();
}

} // namespace

proc_def_t accel_cmd = {"accel", HIGH1, 128, accel};

} // namespace shell
