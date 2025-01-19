#pragma once

#include "types.h"

struct recover_t {
  uint32_t magic;
};

bool is_reset();
void set_magic();
