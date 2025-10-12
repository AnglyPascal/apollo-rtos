#include "core/test.h"
#include "core/boot.h"
#include "core/recover.h"
#include "core/sched.h"
#include "utility/debug.h"

#include "enabled_test_suites.h"

namespace tests
{
SEC_ADDR(unit_tests);
SEC_ADDR(sys_tests);

size_t i_unit __recover_section__ = 0;
size_t i_sys __recover_section__ = 0;
size_t n_unit_tests __recover_section__ = 0;
size_t n_sys_tests __recover_section__ = 0;

constexpr size_t n_suites = sizeof(test_suites) / sizeof(test_suites[0]);

inline bool suite_enabled(const char *suite)
{
  // enable all tests by default
  if (n_suites == 0)
    return true;

  for (size_t i = 0; i < n_suites; ++i)
    if (strcmp(suite, test_suites[i]) == 0)
      return true;
  return false;
}

inline void report()
{
  clear_line();
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
        ? debug<TRACE>(BLUE "%s.%s: " DEFAULT GREEN "Passed" DEFAULT "\r\n",
                       test->suite, test->name)
        : debug<ERROR>(BOLD BLUE "%s.%s: " DEFAULT RED "Failed" DEFAULT "\r\n",
                       test->suite, test->name);

    passed_tests += *test->passed;
    total_tests++;
  };

  kprintf("\r\n" BOLD "Unit test results:" DEFAULT "\r\n");
  for (SEC_ITER(unit_tests, const test_t, test)) {
    if (!suite_enabled(test->suite))
      continue;
    result(test);
  }
  total_result();

  sched::yield();

  passed_tests = 0;
  total_tests = 0;

  kprintf("\r\n" BOLD "System test results:" DEFAULT "\r\n");
  for (SEC_ITER(sys_tests, const test_t, test)) {
    if (!suite_enabled(test->suite))
      continue;
    result(test);
  }
  total_result();

  kprintf("\r\n");
}

inline void run_tests(test_t *tests, size_t &idx, size_t n_tests)
{
  while (idx < n_tests) {
    auto [suite, name, func, passed] = tests[idx++];
    if (!suite_enabled(suite))
      continue;

    clear_line();
    debug<ERROR>(DEFAULT "running %s.%s: ", suite, name);
    *passed = func();
    debug<ERROR>("%s\n\r" DEFAULT, *passed ? GREEN "passed" : RED "FAILED");
    trigger_reset();
  }
}

PROC(sys_tests, HIGHEST, 1024, param)
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

    if (n_suites == 0)
      debug<INFO>("running " BLUE "all" DEFAULT " suites\r\n\r\n");
    else {
      debug<INFO>("running suites: ");
      for (size_t i = 0; i < n_suites - 1; i++)
        debug<INFO>(YELLOW "%s" DEFAULT ", ", test_suites[i]);
      debug<INFO>(YELLOW "%s" DEFAULT "\r\n\r\n", test_suites[n_suites - 1]);
    }
  }

  run_tests((test_t *)SEC_START(unit_tests), i_unit, n_unit_tests);
  REG_PROC(sys_tests, nullptr);
}

} // namespace tests
