#include "core/memory.h"
#include "core/recover.h"
#include "fs/fs.h"
#include "utility/debug.h"

#define RAM_MAGIC 0xbabedadd

__extern_C__
byte_t __recover_load[],
    __recover_start[], __recover_end[];

namespace fs
{
bool first_boot();
}

namespace boot
{
namespace
{
boot_lev_t boot_lev = boot_lev_t::FLASH;
uint32_t ram_magic __recover_section__ = RAM_MAGIC;
} // namespace

boot_lev_t lev() { return boot_lev; }

const char *lev_str()
{
  static const char *levs[] = {
      "reset",
      "boot",
      "power",
      "flash",
  };
  return levs[static_cast<uint8_t>(lev())];
}

void init()
{
  bool ram_reset = ram_magic != RAM_MAGIC;
  ram_magic = RAM_MAGIC;

  if (fs::first_boot())
    boot_lev = boot_lev_t::FLASH;
  else if (POWER.RESETREAS == 0)
    boot_lev = boot_lev_t::POWER;
  else
    boot_lev = ram_reset ? boot_lev_t::BOOT : boot_lev_t::RESET;

  POWER.RESETREAS = 0xFFFFFFFF;

  kprintf("boot level: %s\r\n", boot::lev_str());

  // initialize recovery section
  if (boot::lev() != boot_lev_t::RESET)
    _memcpy(__recover_start, __recover_load, __recover_end - __recover_start);
}
} // namespace boot
