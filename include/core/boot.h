#pragma once

#include "types.h"

enum class boot_lev_t {
  RESET,
  BOOT,
  POWER,
  FLASH,
};

struct boot_stat_t {
  uint32_t n_reset = 0;
  uint32_t n_boot = 0;
  uint32_t n_flash = 0;
  uint32_t n_power = 0;

  void incr(boot_lev_t lev)
  {
    if (lev == boot_lev_t::RESET)
      n_reset++;
    else if (lev == boot_lev_t::BOOT)
      n_boot++;
    else if (lev == boot_lev_t::POWER)
      n_power++;
    else
      n_flash++;
  }
};

namespace boot
{
bool first_boot();
boot_lev_t lev();
const char *lev_str();
void init();
} // namespace boot
