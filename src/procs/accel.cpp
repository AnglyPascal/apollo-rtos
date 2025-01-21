#include "circular_buffer.h"
#include "fs.h"
#include "nvm.h"
#include "recover.h"
#include "sched.h"
#include "serial.h"
#include "waitlist.h"

namespace procs
{

void *accel(void *param);

namespace
{
struct accel_t {
  int x;
  int y;
  int z;
};

constexpr size_t len = (pg_sz - circular_buffer_header_sz) / sizeof(accel_t);
using buffer = circular_buffer<accel_t, len>;

int n __recover_section__ = 0;

void *test_recover_func(void *param)
{
  sched::reg_proc("accel", 32, 256, accel, nullptr);
  return param;
}

} // namespace

fn_t accel_fn = 5;

void *accel(void *param)
{
  recovery::alloc(test_recover_func, nullptr);

  auto file = fs::open(accel_fn, sizeof(buffer), O_WRITE | O_CREATE);
  buffer *val = (buffer *)*file;

  /* int n = 0; */
  while (1) {
    n++;
    val->enqueue(n, n, n);

    /* if ((n & 31) == 0) */
    /*   file->store(); */

    sched::sleep(1000);
  }

  return param;
}

} // namespace procs
