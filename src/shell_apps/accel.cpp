#include "accel.h"
#include "char_buffer.h"
#include "circular_buffer.h"
#include "display.h"
#include "fs.h"
#include "memory.h"
#include "nvm.h"
#include "serial.h"
#include "shell.h"
#include "types.h"

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

const image_t dirs[] = {
    IMAGE(0, 0, 1, 0, 0,  //
          0, 0, 1, 0, 0,  //
          0, 0, 1, 0, 0,  //
          0, 0, 0, 0, 0,  //
          0, 0, 0, 0, 0), //
                          //
    IMAGE(0, 0, 0, 0, 1,  //
          0, 0, 0, 1, 0,  //
          0, 0, 1, 0, 0,  //
          0, 0, 0, 0, 0,  //
          0, 0, 0, 0, 0), //
                          //
    IMAGE(0, 0, 0, 0, 0,  //
          0, 0, 0, 0, 0,  //
          0, 0, 1, 1, 1,  //
          0, 0, 0, 0, 0,  //
          0, 0, 0, 0, 0), //
                          //
    IMAGE(0, 0, 0, 0, 0,  //
          0, 0, 0, 0, 0,  //
          0, 0, 1, 0, 0,  //
          0, 0, 0, 1, 0,  //
          0, 0, 0, 0, 1), //
                          //
    IMAGE(0, 0, 0, 0, 0,  //
          0, 0, 0, 0, 0,  //
          0, 0, 1, 0, 0,  //
          0, 0, 1, 0, 0,  //
          0, 0, 1, 0, 0), //
                          //
    IMAGE(0, 0, 0, 0, 0,  //
          0, 0, 0, 0, 0,  //
          0, 0, 1, 0, 0,  //
          0, 1, 0, 0, 0,  //
          1, 0, 0, 0, 0), //
                          //
    IMAGE(0, 0, 0, 0, 0,  //
          0, 0, 0, 0, 0,  //
          1, 1, 1, 0, 0,  //
          0, 0, 0, 0, 0,  //
          0, 0, 0, 0, 0), //
                          //
    IMAGE(1, 0, 0, 0, 0,  //
          0, 1, 0, 0, 0,  //
          0, 0, 1, 0, 0,  //
          0, 0, 0, 0, 0,  //
          0, 0, 0, 0, 0), //
                          //
    IMAGE(0, 0, 0, 0, 0,  //
          0, 0, 0, 0, 0,  //
          0, 0, 1, 0, 0,  //
          0, 0, 0, 0, 0,  //
          0, 0, 0, 0, 0), //
};

void *accel(void *param)
{
  auto file = fs::open(accel::fn, sizeof(buffer), O_CREATE);
  auto val = (buffer *)*file;

  serial::listener_guard guard{listener};

  int threshold = 10;
  auto is_zero = [threshold](int p) {
    return (-threshold < p) && (p < threshold);
  };

  while (!exit) {
    serial::clear_screen();

    if (!val->empty()) {
      auto [x, y, z] = val->back();

      int dir;
      if (is_zero(x)) {
        dir = is_zero(y) ? 8 : (y > 0 ? 0 : 4);
      } else if (x > 0) {
        dir = is_zero(y) ? 2 : (y > 0 ? 1 : 3);
      } else {
        dir = is_zero(y) ? 6 : (y > 0 ? 7 : 5);
      }

      display::show(dirs[dir]);
      printf("x: %d, y: %d, z: %d\r\n", x, y, z);
    }

    sched::sleep(100);
  }

  exit = false;
  serial::clear_screen();
  display::reset();

  return param;
}

} // namespace

proc_def_t accel_cmd = {"accel", 30, 128, accel, nullptr};

} // namespace shell
