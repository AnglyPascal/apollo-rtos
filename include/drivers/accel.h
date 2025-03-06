#pragma once

#include "core/types.h"
#include "fs/fs.h"

namespace accel
{
inline constexpr fn_t fn = 5;

void init();
void read(int *x, int *y, int *z);
} // namespace accel
