#include "fs/fs.h"

#include "core/hardware.h"

/** If CPU is halted during write/erase operations, why do we need to busy wait?
 */
__always_inline__ inline void wait()
{
  while (!NVMC.READY)
    ;
}

namespace _flash
{
void erase(addr_t pg_addr)
{
  NVMC.CONFIG = NVMC_CONFIG_EEN;
  wait();
  NVMC.ERASEPAGE = (void *)pg_addr;
  wait();
  NVMC.CONFIG = NVMC_CONFIG_REN;
  wait();
}

size_t write(addr_t addr, uint8_t *buf, size_t sz)
{
  erase(addr);

  NVMC.CONFIG = NVMC_CONFIG_WEN;
  wait();

  auto pg_addr = (word_t *)addr;
  auto rt_addr = (word_t *)buf;

  for (size_t i = 0; i < sz / sizeof(uint32_t); i++) {
    pg_addr[i] = rt_addr[i];
    wait();
  }

  NVMC.CONFIG = NVMC_CONFIG_REN;
  wait();

  return sz;
}

size_t read(addr_t addr, uint8_t *buf, size_t sz)
{
  auto pg_addr = (word_t *)addr;
  auto rt_addr = (word_t *)buf;

  for (size_t i = 0; i < sz / sizeof(uint32_t); i++) {
    rt_addr[i] = pg_addr[i];
    wait();
  }

  return sz;
}

} // namespace _flash
