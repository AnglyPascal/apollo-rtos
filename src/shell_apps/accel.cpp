#include "char_buffer.h"
#include "circular_buffer.h"
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

fn_t accel_fn = 5;

void *accel(void *param)
{
  auto file = fs::open(accel_fn, sizeof(buffer), O_CREATE);
  auto val = (buffer *)*file;

  serial::listener_guard guard{listener};

  while (!exit) {
    serial::clear_screen();

    if (!val->empty()) {
      auto [x, y, z] = val->back();
      printf("x: %d, y: %d, z: %d\r\n", x, y, z);
    }

    sched::sleep(1000);
  }

  exit = false;
  serial::clear_screen();

  return param;
}

} // namespace

proc_def_t accel_cmd = {"accel", 4, 128, accel, nullptr};

} // namespace shell
