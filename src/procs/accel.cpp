#include "fs.h"
#include "nvm.h"
#include "sched.h"
#include "serial.h"
#include "waitlist.h"

namespace procs
{

namespace
{
struct accel_t {
  int x;
  int y;
  int z;
};
} // namespace

fd_t accel_fd = 5;

void *accel(void *param)
{
  auto file = fs::open(accel_fd, sizeof(accel_t), O_WRITE | O_CREATE);
  accel_t *val = (accel_t *)**file;

  int n = 0;
  while (1) {
    n++;
    *val = {n, n, n};
    file->store();
    sched::sleep(1000);
  }

  fs::close(file);
  return param;
}

} // namespace procs
