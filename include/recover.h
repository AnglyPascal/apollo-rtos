#pragma once

#include "types.h"

#define __recover_section__ __attribute__((section(".recover"))) __attribute__((__used__))

bool is_reset();
void set_magic();

namespace recovery
{
void alloc(runnable_t recover_func, void *param);
void dealloc();
} // namespace recovery

void recover();
