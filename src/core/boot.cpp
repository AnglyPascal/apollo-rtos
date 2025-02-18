#include "core/memory.h"
#include "core/recover.h"
#include "utility/debug.h"

#define REC_MAGIC 0xbabedadd

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
bool power_on = false;
uint32_t power_magic __recover_section__ = REC_MAGIC;
} // namespace

void init()
{
  power_on = power_magic != REC_MAGIC;
  power_magic = REC_MAGIC;

  auto _lev = lev();
  if (_lev == boot_lev_t::RESET)
    kprintf("reset\r\n");
  else if (_lev == boot_lev_t::FLASH)
    kprintf("flash\r\n");
  else
    kprintf("boot\r\n");

  // initialize recovery section
  if (boot::lev() != boot_lev_t::RESET)
    _memcpy(__recover_start, __recover_load, __recover_end - __recover_start);
}

boot_lev_t lev()
{
  if (fs::first_boot())
    return boot_lev_t::FLASH;
  return power_on ? boot_lev_t::BOOT : boot_lev_t::RESET;
}
} // namespace boot
