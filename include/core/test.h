#pragma once

#include "core/recover.h"
#include "core/types.h"

using test_func_t = bool (*)(void);

struct test_t {
  const char *name;
  test_func_t func;
  bool *passed;
};

#define TEST_MACRO(sec, name)                                                  \
  bool __##name##_test_func(void);                                             \
  bool __##name##_passed __recover_section__ = false;                          \
  const test_t __attribute__((section(#sec), __used__)) __##name##_test_obj{   \
      #name, __##name##_test_func, &__##name##_passed};                        \
  bool __##name##_test_func(void)

#define TEST(...) TEST_MACRO(.unit_tests, ##__VA_ARGS__)
#define SYS_TEST(...) TEST_MACRO(.sys_tests, ##__VA_ARGS__)

#define _TEST(name)                                                            \
  bool __##name##_test_unused(void) __attribute__((unused));                   \
  bool __##name##_test_unused(void)

namespace tests
{
void run();
}
