#pragma once

#include "core/types.h"
#include "fs/fs.h"

namespace accel
{
inline constexpr fn_t fn = 5;

struct data_t {
  int8_t x;
  int8_t y;
  int8_t z;
};

constexpr size_t len = 8;
using buffer = circular_buffer<data_t, len>;

void init();
void read(data_t &data);
} // namespace accel
