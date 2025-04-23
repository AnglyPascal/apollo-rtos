#pragma once

#include "types.h"

enum class boot_lev_t {
  RESET,
  BOOT,
  POWER,
  FLASH,
};

namespace boot
{
boot_lev_t lev();
bool is_boot();

const char *lev_str();
void stat();

void init();
} // namespace boot
