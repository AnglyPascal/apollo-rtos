#include "core/irq.h"
#include "core/recover.h"
#include "core/sched.h"

__extern_C__
void trigger_reset();

namespace procs
{

void test_proc(void *param);

proc_def_t test{"test_proc", MID1, 64, test_proc};

void test_proc(void *param)
{
  recover::set_rec(rec_lev_t::RESET);

  delay_loop(1000);
  sched::decr_priority(LOW4);

  while (1) {
    delay_loop(10000);
    sched::sleep(50);
  }
}

} // namespace procs
