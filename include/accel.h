#pragma once

#include "types.h"
#include "fs.h"

namespace accel
{
inline constexpr fn_t fn = 5;

void init();
void read(int *x, int *y, int *z);
} // namespace accel
