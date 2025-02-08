#include "drivers/flash.h"

#include "core/hardware.h"
#include "utility/debug.h"

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

void *nvm_t::operator*() const { return rt_addr; }

#define PG_MAGIC 0xbebebebe;

void pg_t::load() const
{
  if (is_valid())
    nvm_t::load();
}

bool pg_t::is_valid() const
{
  auto addr = (page_guard_t *)((uint32_t)pg_addr + pg_sz) - 1;
  assert((uint32_t)addr % sizeof(word_t) == 0);
  return *addr == PG_MAGIC;
}

void pg_t::store() const
{
  nvm_t::store();
  auto addr = (page_guard_t *)((uint32_t)pg_addr + pg_sz) - 1;
  *addr = PG_MAGIC;
}
