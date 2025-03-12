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

int n __recover_section__ = 100;
} // namespace

void accel_func(void *param)
{
  recover::guard_proc guard{POWER_OFF};

  auto file =
      fram::open<accel::buffer>(accel::fn, O_WRITE | O_CREATE | O_SHARED);
  auto buf = file.mmap<accel::buffer>();

  accel::init();

  uint8_t n = MAX<uint8_t>;
  accel::data_t data;
  while (!curr_proc::term_req()) {
    accel::read(data);
    buf->enqueue(data);

    if (n-- == 0)
      file.store();

    sched::sleep(200);
  }
  file.store();
}

} // namespace procs
