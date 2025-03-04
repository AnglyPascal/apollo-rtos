#include "drivers/accel.h"
#include "core/fs.h"
#include "core/recover.h"
#include "core/sched.h"
#include "drivers/serial.h"
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

constexpr size_t len = (pg_sz - circular_buffer_header_sz) / sizeof(accel_t);
using buffer = circular_buffer<accel_t, len>;

int n __recover_section__ = 100;
} // namespace

void accel_func(void *param)
{
  recover::guard_proc guard{BOOT};

  auto file = flash::open(accel::fn, sizeof(buffer), O_WRITE | O_CREATE);
  auto ptr = flash::mmap<buffer>(file);
  auto val = new (ptr) buffer{};

  accel::init();

  int x, y, z;
  while ((volatile int)1) {
    accel::read(&x, &y, &z);
    val->enqueue(x, y, z);

    /* if ((n & 31) == 0) */
    /*   file->store(); */

    sched::sleep(200);
  }
}

} // namespace procs
