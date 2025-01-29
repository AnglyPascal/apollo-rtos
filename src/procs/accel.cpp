#include "accel.h"
#include "circular_buffer.h"
#include "fs.h"
#include "irq.h"
#include "nvm.h"
#include "recover.h"
#include "sched.h"
#include "serial.h"
#include "waitlist.h"

namespace procs
{

void *accel_func(void *param);

proc_def_t accel{"accel_bg", 32, 256, accel_func, nullptr};

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

__attribute__((optimize("O0"))) // O2 doesn't work
void *accel_func(void *param)
{
  recovery::set_rec_lev(rec_lev_t::RESET);
  recovery::store_data(accel);

  accel::init();

  auto file = fs::open(accel::fn, sizeof(buffer), O_WRITE | O_CREATE);
  auto val = file.is_valid() ? (buffer *)*file : new (*file) buffer{};

  int x, y, z;
  while (1) {
    asm volatile("" ::: "memory");

    accel::read(&x, &y, &z);
    val->enqueue(x, y, z);

    /* if ((n & 31) == 0) */
    /*   file->store(); */

    sched::sleep(200);
  }

  return param;
}

} // namespace procs
