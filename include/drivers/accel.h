#pragma once

#include "core/types.h"
#include "fs/fs.h"

namespace accel
{
struct data_t {
  int8_t x;
  int8_t y;
  int8_t z;
};

void init();
data_t read();
} // namespace accel
