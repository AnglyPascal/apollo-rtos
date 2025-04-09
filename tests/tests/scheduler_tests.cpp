#include "core/sched.h"
#include "core/test.h"
#include "drivers/display.h"
#include "utility/debug.h"

namespace
{
uint32_t results[3];
size_t idx = 0;

PROC_MANUAL(proc0, LOW2, 8, param) { results[idx++] = *(uint32_t *)param; }
PROC_MANUAL(proc1, MID2, 8, param) { results[idx++] = *(uint32_t *)param; }
PROC_MANUAL(proc2, HIGH2, 8, param) { results[idx++] = *(uint32_t *)param; }

SYS_TEST(sched_basic_priority)
{
  sched::reg_proc(&PROC_DEF(proc0), 2);
  sched::reg_proc(&PROC_DEF(proc1), 1);
  sched::reg_proc(&PROC_DEF(proc2), 0);

  // FIXME: write a proper await mechanism
  sched::decr_priority(LOW1);
  while (idx != 3)
    ;
  sched::incr_priority(HIGHEST);

  return results[0] == 0 && results[1] == 1 && results[2] == 2;
}
} // namespace
