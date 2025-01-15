#include "display.h"
#include "sched.h"

namespace procs
{

void *proc1(void *);
void *proc2(void *);
void *accel(void *);

} // namespace procs

namespace sched
{

void setup_procs(void)
{
  sched::reg_proc("proc1", 7, 0, procs::proc1, nullptr);
  sched::reg_proc("proc2", 10, 64, procs::proc2, nullptr);
  sched::reg_proc("accel", 32, 64, procs::accel, nullptr);
  display::init();
}

} // namespace sched
