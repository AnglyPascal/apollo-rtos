#include "core/sched.h"
#include "drivers/display.h"

namespace procs
{
extern proc_def_t test, accel, display;

namespace
{
proc_def_t *proc_defs[] = {
    &test,
    &accel,
    &display,
};
}

} // namespace procs

namespace sched
{
void setup_procs(void)
{
  for (auto proc_def : procs::proc_defs) {
    reg_proc(proc_def, nullptr);
  }
}
} // namespace sched
