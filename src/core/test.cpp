#include "core/test.h"
#include "core/boot.h"
#include "core/memory.h"
#include "core/recover.h"
#include "utility/debug.h"

namespace tests
{
SECTION_ADDR(tests);

int idx __recover_section__ = 0;

void run()
{
  SECTION_INIT(tests);

  int i = 0;
  for (SECTION_ITER(tests, test_t, test)) {
    if (i++ != idx)
      continue;

    auto [name, func, passed] = *test;
    *passed = func();

    idx++;
    trigger_reset();
  }

  boot::stat();
  kprintf("\r\n" BOLD "Test results:" DEFAULT "\r\n");

  for (SECTION_ITER(tests, const test_t, test)) {
    auto [name, func, passed] = *test;

    if (*passed)
      kprintf(BLUE "%s: " DEFAULT GREEN "Passed" DEFAULT "\r\n", name);
    else
      kprintf(BOLD BLUE "%s: " DEFAULT RED "Failed" DEFAULT "\r\n", name);
  }
}

} // namespace tests
