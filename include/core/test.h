#pragma once

#include "core/types.h"

using test_func_t = bool (*)(void);

struct test_t {
  const char *name;
  test_func_t func;
};

#define TEST(name)                                                             \
  bool __##name##_test_func(void);                                             \
  const test_t                                                                 \
      __attribute__((section(".tests"), __used__)) __##name##_test_obj{        \
          #name, __##name##_test_func};                                        \
  bool __##name##_test_func(void)

