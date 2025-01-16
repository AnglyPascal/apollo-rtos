#include "circular_buffer.h"
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

constexpr size_t len = (pg_sz - circular_buffer_header_sz) / sizeof(accel_t);
using buffer = circular_buffer<accel_t, len>;
} // namespace

fd_t accel_fd = 5;

void *accel(void *param)
{
  auto file = fs::open(accel_fd, sizeof(buffer), O_WRITE | O_CREATE);
  buffer *val = (buffer *)**file;

  int n = 0;
  while (1) {
    n++;
    val->enqueue(n, n, n);

    if ((n & 3) == 0)
      file->store();

    sched::sleep(1000);
  }

  fs::close(file);
  return param;
}

} // namespace procs
