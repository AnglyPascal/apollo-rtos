#include "core/fs.h"
#include "core/irq.h"
#include "core/recover.h"
#include "core/sched.h"
#include "drivers/i2c.h"

namespace procs
{

void test_proc(void *param);

proc_def_t test{"test_proc", MID1, 64, test_proc};

void single_blk_file_rw()
{
  auto file = fram::open(10, 16, O_WRITE | O_CREATE);
  auto t = (char *)fram::mmap(file, 16);
  kprintf("fram read: %s\r\n", t);

  auto s = "It IS a string";
  while (*s != '\0')
    *t++ = *s++;
  *t = '\0';

  fram::store(file);
}

void multi_blk_file_rw()
{
  auto file = fram::open(12, 256, O_WRITE | O_CREATE);
  auto t = (uint32_t *)fram::mmap(file, 256);

  kprintf("fram read: ");
  for (size_t i = 0; i < 4; i++)
    kprintf("%x, ", t[i]);
  kprintf("%x\r\n", t[4]);

  for (size_t i = 0; i < 256 / sizeof(*t); i++) {
    *t++ = 0xABCD0123;
  }

  fram::store(file);
}

void test_proc(void *param)
{
  recover::set_rec(rec_lev_t::RESET);

  // FIXME: move from here
  i2c::scan();

  single_blk_file_rw();
  multi_blk_file_rw();

  delay_loop(1000);
  sched::decr_priority(LOW4);

  while (1) {
    delay_loop(10000);
    sched::sleep(50);
  }
}

} // namespace procs
