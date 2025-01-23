#include "recover.h"
#include "sched.h"

__extern_C__
void trigger_reset();

namespace procs
{

void *test_proc(void *param);

proc_def_t test{"test_proc", 10, 64, test_proc, nullptr};

void *test_proc(void *param)
{
  recovery::set_rec_lev(rec_lev_t::RESET);
  recovery::store_data(test);

  int n = 3;
  while (n-- > 0)
    ;
  sched::decr_priority(5);

  while (1) {
    int m = 10;
    while (m-- > 0) {
      n = 100000000;
      while (n-- > 0)
        ;
    }

    sched::sleep(50);
  }

  return param;
}

} // namespace procs
