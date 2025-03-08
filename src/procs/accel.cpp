#include "drivers/accel.h"
#include "core/recover.h"
#include "core/sched.h"
#include "drivers/serial.h"
#include "fs/fs.h"
#include "utility/circular_buffer.h"

namespace procs
{

void accel_func(void *param);

proc_def_t accel{"accel_bg", HIGH3, 256, accel_func};

namespace
{
struct accel_t {
  int x;
  int y;
  int z;
};

constexpr size_t len = 8;
using buffer = circular_buffer<accel_t, len>;

int n __recover_section__ = 100;
} // namespace

void accel_func(void *param)
{
  recover::guard_proc guard{BOOT};

  auto file =
      fram::open(accel::fn, sizeof(buffer), O_WRITE | O_CREATE | O_SHARED);
  auto ptr = file.mmap<buffer>();
  auto val = new (ptr) buffer{};

  accel::init();

  uint8_t n = MAX<uint8_t>;
  int x, y, z;
  while (!curr_proc::term_req()) {
    accel::read(&x, &y, &z);
    val->enqueue(x, y, z);

    if (n-- == 0)
      file.store();

    sched::sleep(200);
  }
  file.store();
}

} // namespace procs
