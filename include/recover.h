#pragma once

#include "types.h"

bool is_reset();
void set_magic();

namespace recovery
{
void alloc(runnable_t recover_func, void *param);
void dealloc();
} // namespace recovery

void recover();
