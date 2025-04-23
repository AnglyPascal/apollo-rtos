#pragma once

#include "core/recover.h"
#include "core/types.h"

using test_func_t = bool (*)(void);

struct test_t {
  const char *suite;
  const char *name;
  test_func_t func;
  bool *passed;
};

#define TEST_MACRO(sec, name)                                                  \
  bool __##name##_test_func(void);                                             \
  bool __##name##_passed __recover_section__ = false;                          \
  const test_t __attribute__((section(#sec), __used__)) __##name##_test_obj{   \
      SUITE_NAME, #name, __##name##_test_func, &__##name##_passed};            \
  bool __##name##_test_func(void)

#define TEST(...) TEST_MACRO(.unit_tests, ##__VA_ARGS__)
#define SYS_TEST(...) TEST_MACRO(.sys_tests, ##__VA_ARGS__)

#define _TEST(name)                                                            \
  bool __##name##_test_unused(void) __attribute__((unused));                   \
  bool __##name##_test_unused(void)

#define _SYS_TEST(name)                                                            \
  bool __##name##_test_unused(void) __attribute__((unused));                   \
  bool __##name##_test_unused(void)

#define TEST_SUITE(name) __##name##_tests

namespace tests
{
void run();
}

#define BEGIN_SUITE(name)                                                      \
  namespace __##name##_tests                                                   \
  {                                                                            \
    inline const char *const SUITE_NAME = #name;

#define END_SUITE() }
