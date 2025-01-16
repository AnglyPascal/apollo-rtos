#pragma once

#include "debug.h"
#include "memory.h"
#include "types.h"

static constexpr size_t pg_sz = 1024;

// runtime representation of a nvm page
class nvm_t
{
  uint32_t *const pg_addr;
  uint32_t *const rt_addr;
  size_t sz;

public:
  constexpr nvm_t(uint32_t *pg_addr, uint32_t *rt_addr, size_t sz)
      : pg_addr{pg_addr}, rt_addr{rt_addr}, sz{sz}
  {
    /* assert(((uint32_t)pg_addr & (sizeof(uint32_t) - 1)) == 0); */
    /* assert((sz & (sizeof(uint32_t) - 1)) == 0); */
  }

  nvm_t(uint32_t *pg_addr, size_t sz)
      : nvm_t(pg_addr, (uint32_t *)heap::malloc(sz), sz)
  {
    load();
  }

  void load() const;
  void erase() const;
  void store() const;
  void *operator*() const;

  ~nvm_t();
};
