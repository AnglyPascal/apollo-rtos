#include "core/test.h"
#include "core/boot.h"
#include "core/memory.h"
#include "core/recover.h"
#include "core/sched.h"
#include "utility/debug.h"

namespace tests
{
SEC_ADDR(unit_tests);
SEC_ADDR(sys_tests);

size_t i_unit __recover_section__ = 0;
size_t i_sys __recover_section__ = 0;
size_t n_unit_tests __recover_section__ = 0;
size_t n_sys_tests __recover_section__ = 0;

inline void report()
{
  boot::stat();

  int passed_tests = 0;
  int total_tests = 0;

  auto total_result = [&]() {
    (passed_tests == total_tests)
        ? kprintf(GREEN "all")
        : kprintf(BOLD RED "(%d/%d)", passed_tests, total_tests);
    kprintf(" tests passed" DEFAULT "\r\n");
  };

  auto result = [&](auto test) {
    (*test->passed)
        ? kprintf(BLUE "%s: " DEFAULT GREEN "Passed" DEFAULT "\r\n", test->name)
        : kprintf(BOLD BLUE "%s: " DEFAULT RED "Failed" DEFAULT "\r\n",
                  test->name);

    passed_tests += *test->passed;
    total_tests++;
  };

  kprintf("\r\n" BOLD "Unit test results:" DEFAULT "\r\n");
  for (SEC_ITER(unit_tests, const test_t, test))
    result(test);
  total_result();

  passed_tests = 0;
  total_tests = 0;

  kprintf("\r\n" BOLD "System test results:" DEFAULT "\r\n");
  for (SEC_ITER(sys_tests, const test_t, test))
    result(test);
  total_result();

  kprintf("\r\n");
}

inline void run_tests(test_t *tests, size_t &idx, size_t n_tests)
{
  while (idx < n_tests) {
    auto [name, func, passed] = tests[idx++];
    debug<ERROR>("running test %s\r\n", name);
    *passed = func();
    trigger_reset();
  }
}

PROC(sys_tests, HIGHEST, 512, param)
{
  run_tests((test_t *)SEC_START(sys_tests), i_sys, n_sys_tests);
  report();
}

void run()
{
  n_unit_tests = SEC_LENGTH(unit_tests, test_t);
  n_sys_tests = SEC_LENGTH(sys_tests, test_t);

  if (n_unit_tests == 0 && n_sys_tests == 0)
    return boot::stat();

  if (boot::lev() != boot_lev_t::RESET) {
    SEC_INIT(unit_tests);
    SEC_INIT(sys_tests);
  }

  run_tests((test_t *)SEC_START(unit_tests), i_unit, n_unit_tests);
  REG_PROC(sys_tests, nullptr);
}

} // namespace tests
