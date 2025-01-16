#include "char_buffer.h"
#include "fs.h"
#include "memory.h"
#include "nvm.h"
#include "serial.h"
#include "shell.h"
#include "types.h"

namespace shell
{

namespace
{

struct accel_t {
  int x;
  int y;
  int z;
};

volatile bool exit = false;

bool listener(char c)
{
  if (c == CTRL('q')) {
    exit = true;
    return true;
  }
  return false;
}

fd_t accel_fd = 5;

void *accel(void *param)
{
  auto file = fs::open(accel_fd, sizeof(accel_t), O_CREATE);
  auto val = (accel_t *)**file;
  serial::listener_guard guard{listener};

  while (!exit) {
    serial::clear_screen();
    file->load();
    printf("x: %d, y: %d, z: %d\r\n", val->x, val->y, val->z);
    sched::sleep(1000);
  }

  exit = false;
  serial::clear_screen();
  fs::close(file);

  return param;
}

} // namespace

cmd_t accel_cmd = {"accel", 4, 128, accel};

} // namespace shell
