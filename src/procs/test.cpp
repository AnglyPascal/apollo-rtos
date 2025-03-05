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
  auto t = (char *)file.mmap(16);
  kprintf("fram read: %s\r\n", t);

  auto s = "It IS a string";
  while (*s != '\0')
    *t++ = *s++;
  *t = '\0';

  file.store();
}

void multi_blk_file_rw()
{
  auto file = fram::open(12, 256, O_WRITE | O_CREATE);
  auto t = (uint32_t *)file.mmap(256);

  kprintf("fram read: ");
  for (size_t i = 0; i < 4; i++)
    kprintf("%x, ", t[i]);
  kprintf("%x\r\n", t[4]);

  for (size_t i = 0; i < 256 / sizeof(*t); i++) {
    *t++ = 0xABCD0123;
  }

  file.store();
}

__extern_C__
void trigger_reset(void);

int n_run __recover_section__ = 0;

void reset_task_test()
{
  auto task = [](void *param) {
    n_run = *(int *)param;
    n_run++;
  };

  recover::guard_task guard{RESET, task, 0, n_run};
  if (n_run < 1) {
    trigger_reset();
  }
}

void test_proc(void *param)
{
  recover::guard_proc guard{RESET};

  // FIXME: move from here
  /* i2c::scan(); */

  single_blk_file_rw();
  multi_blk_file_rw();

  delay_loop(1000);
  sched::decr_priority(LOW1);

  reset_task_test();

  while (1) {
    delay_loop(10000);
    sched::sleep(50);
  }
}

} // namespace procs
