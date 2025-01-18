#include "nvm.h"
#include "debug.h"
#include "hardware.h"
#include "memory.h"

/** If CPU is halted during write/erase operations, why do we need to busy wait?
 */
__always_inline__
inline void wait()
{
  while (!NVMC.READY)
    ;
}

void nvm_t::load() const
{
  for (size_t i = 0; i < sz / sizeof(uint32_t); i++) {
    rt_addr[i] = pg_addr[i];
    wait();
  }
}

void nvm_t::erase() const
{
  NVMC.CONFIG = NVMC_CONFIG_EEN;
  wait();
  NVMC.ERASEPAGE = pg_addr;
  wait();
  NVMC.CONFIG = NVMC_CONFIG_REN;
  wait();
}

void nvm_t::store() const
{
  erase();

  NVMC.CONFIG = NVMC_CONFIG_WEN;
  wait();

  for (size_t i = 0; i < sz / sizeof(uint32_t); i++) {
    pg_addr[i] = rt_addr[i];
    wait();
  }

  NVMC.CONFIG = NVMC_CONFIG_REN;
  wait();
}

void *nvm_t::operator*() const
{
  return rt_addr;
}

