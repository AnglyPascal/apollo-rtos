#include "core/sched.h"
#include "core/test.h"
#include "drivers/display.h"
#include "utility/debug.h"

PROC(dance, MID4, 128, p)
{
  int i = 0;
  while (1) {
    display::image_set(i % 5, i / 5);
    sched::sleep(100);

    i = (i + 1) % 25;
    if (i == 0)
      display::reset();
  }
}

namespace sched
{
void setup_services(void) {}
void setup_procs(void) {}
} // namespace sched
