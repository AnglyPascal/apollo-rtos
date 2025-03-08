#include "core/boot.h"
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

constexpr fn_t boot_fn = 2;

void init()
{
  bool ram_reset = ram_magic != RAM_MAGIC;
  ram_magic = RAM_MAGIC;

  if (fs::first_boot())
    boot_lev = boot_lev_t::FLASH;
  else if (POWER.RESETREAS == 0)
    boot_lev = boot_lev_t::POWER;
  else if (ram_reset)
    boot_lev = boot_lev_t::BOOT;
  else
    boot_lev = boot_lev_t::RESET;

  auto file = fram::open<boot_stat_t>(boot_fn, O_WRITE | O_CREATE | O_PERM);
  auto stat = file.mmap<boot_stat_t>();
  stat->incr(boot_lev);
  file.store();

  POWER.RESETREAS = 0xFFFFFFFF;

  kprintf("boot level: %s\r\n", boot::lev_str());
  kprintf(
      "boot stat: n_flash = %d, n_boot = %d, n_power = %d, n_reset = %d\r\n",
      stat->n_flash, stat->n_boot, stat->n_power, stat->n_reset);

  // initialize recovery section
  if (boot::lev() != boot_lev_t::RESET)
    _memcpy(__recover_start, __recover_load, __recover_end - __recover_start);
}
} // namespace boot
