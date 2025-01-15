#include "char_buffer.h"
#include "memory.h"
#include "nvm.h"
#include "serial.h"
#include "shell.h"
#include "types.h"

namespace shell
{

namespace
{

volatile bool exit = false;

bool listener(char c)
{
  if (c == CTRL('q')) {
    exit = true;
    return true;
  }
  return false;
}

void *accel(void *param)
{
  nvm_t nvm{256};
  auto addr = (uint32_t *)*nvm;
  serial::listener_guard guard{listener};

  while (!exit) {
    serial::clear_screen();
    nvm.load();
    printf("x: %d, y: %d, z: %d\r\n", addr[0], addr[1], addr[2]);
    sched::sleep(1000);
  }

  exit = false;
  serial::clear_screen();

  return param;
}

} // namespace

cmd_t accel_cmd = {"accel", 4, 128, accel};

} // namespace shell
