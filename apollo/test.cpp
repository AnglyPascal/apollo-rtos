#include "core/irq.h"
#include "core/recover.h"
#include "core/sched.h"
#include "drivers/i2c.h"
#include "fs/fs.h"
#include "utility/lib.h"

void single_blk_file_rw()
{
  fn_t fn = 10;

  auto file = fram::open(fn, 16, O_WRITE | O_CREATE);
  auto t = (char *)file.mmap(16);

  const char *s = "It IS a string";
  while (*s != '\0')
    *t++ = *s++;
  *t = '\0';

  file.store();
  file.close();
  fram::remove(fn);
}

void multi_blk_file_rw()
{
  fn_t fn = 12;

  auto file = fram::open(fn, 256, O_WRITE | O_CREATE);
  auto t = (uint32_t *)file.mmap(256);

  constexpr uint32_t val = 0xABCD0123;

  bool equal = true;
  for (size_t i = 0; i < 256 / sizeof(*t); i++)
    equal &= t[i] == val;

  for (size_t i = 0; i < 256 / sizeof(*t); i++) {
    *t++ = val;
  }

  file.store();
  file.close();
  fram::remove(fn);
}

int n_run __recover_section__ = 0;

void reset_task_test()
{
  auto task = [](void *p) { (*(int *)p)++; };
  recover::guard_task guard{RESET, task, 0, n_run};
  if (n_run < 1)
    trigger_reset();
}

PROC(test, MID1, 128, param)
{
  recover::guard_proc guard{RESET};

  single_blk_file_rw();
  multi_blk_file_rw();

  delay_loop(1000);
  sched::decr_priority(LOW1);

  /* reset_task_test(); */

  while (!curr_proc::term_req()) {
    delay_loop(10000);
    sched::sleep(500);
  }
}

