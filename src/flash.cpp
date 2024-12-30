#include "flash.h"
#include "hardware.h"

namespace flash
{

inline void wait()
{
  while (!NVMC.READY)
    ;
}

inline uint32_t *pg_addr(uint32_t *addr)
{
  auto pg_sz_msk = FICR.CODEPAGESIZE - 1;
  return addr - ((uint32_t)addr & pg_sz_msk);
}

void erase(uint32_t *pg)
{
  if ((uint32_t)pg & (FICR.CODEPAGESIZE - 1)) {
    // FIXME: panic
    return;
  }

  NVMC.CONFIG = NVMC_CONFIG_EEN;
  wait();
  NVMC.ERASEPAGE = pg;
  wait();
  NVMC.CONFIG = NVMC_CONFIG_REN;
  wait();
}

void write(uint32_t *addr, uint32_t *buffer, size_t sz)
{
  if ((uint32_t)addr & (sizeof(uint32_t) - 1)) {
    // FIXME: panic
    return;
  }

  NVMC.CONFIG = NVMC_CONFIG_WEN;
  wait();

  for (size_t i = 0; i < sz; i++) {
    addr[i] = buffer[i];
    wait();
  }

  NVMC.CONFIG = NVMC_CONFIG_REN;
  wait();
}

} // namespace flash
