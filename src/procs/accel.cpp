#include "drivers/accel.h"
#include "core/fs.h"
#include "core/irq.h"
#include "core/recover.h"
#include "core/sched.h"
#include "core/waitlist.h"
#include "drivers/flash.h"
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

static_assert(sizeof(buffer) <= pg_sz - sizeof(page_guard_t));

int n __recover_section__ = 100;
} // namespace

void accel_func(void *param)
{
  recovery::set_rec_lev(rec_lev_t::RESET);
  recovery::store_data(accel);

  auto file = fs::open(accel::fn, sizeof(buffer), O_WRITE | O_CREATE);
  auto val = file.is_valid() ? (buffer *)*file : new (*file) buffer{};

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
