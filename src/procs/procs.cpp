#include "display.h"
#include "sched.h"

namespace procs
{

void *test_proc(void *);
void *accel(void *);

} // namespace procs

namespace sched
{

void setup_procs(void)
{
  sched::reg_proc("test_proc", 10, 64, procs::test_proc, nullptr);
  sched::reg_proc("accel", 32, 64, procs::accel, nullptr);
  display::init();
}

} // namespace sched
