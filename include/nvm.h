#pragma once

#include "debug.h"
#include "memory.h"
#include "types.h"

// runtime representation of a nvm page
class nvm_t
{
  uint32_t *pg_addr = nullptr;
  uint32_t *rt_addr = nullptr;
  size_t sz = 0;

public:
  nvm_t() {}

  nvm_t(uint32_t *pg_addr, uint32_t *rt_addr, size_t sz)
      : pg_addr{pg_addr}, rt_addr{rt_addr}, sz{sz}
  {
    assert((uint32_t)pg_addr % sizeof(uint32_t) == 0);
    assert(sz % sizeof(uint32_t) == 0, "%d\r\n", sz);
  }

  void load() const;
  void erase() const;
  void store() const;
  void *operator*() const;
};
