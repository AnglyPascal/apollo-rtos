#pragma once

#include "core/recover.h"
#include "core/types.h"

using test_func_t = bool (*)(void);

struct test_t {
  const char *name;
  test_func_t func;
  bool *passed;
};

#define TEST(name)                                                             \
  bool __##name##_test_func(void);                                             \
  bool __##name##_passed __recover_section__ = false;                          \
  const test_t                                                                 \
      __attribute__((section(".tests"), __used__)) __##name##_test_obj{        \
          #name, __##name##_test_func, &__##name##_passed};                    \
  bool __##name##_test_func(void)

#define _TEST(name)                                                            \
  bool __##name##_test_unused(void) __attribute__((unused));                   \
  bool __##name##_test_unused(void)

namespace tests
{
void run();
}
