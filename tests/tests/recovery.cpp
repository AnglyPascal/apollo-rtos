#include "core/irq.h"
#include "core/recover.h"
#include "core/sched.h"
#include "drivers/i2c.h"
#include "fs/fs.h"

int n_run __recover_section__ = 0;

void reset_task_test()
{
  auto task = [](void *p) { (*(int *)p)++; };
  recover::guard_task guard{RESET, task, 0, n_run};
  if (n_run < 1)
    trigger_reset();
}

