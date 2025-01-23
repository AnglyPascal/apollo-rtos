#pragma once

#include "debug.h"
#include "memory.h"
#include "types.h"

// runtime representation of a nvm page
class nvm_t
{
  word_t *pg_addr = nullptr;
  word_t *rt_addr = nullptr;
  size_t sz = 0;

public:
  nvm_t() {}

  nvm_t(word_t *pg_addr, word_t *rt_addr, size_t sz)
      : pg_addr{pg_addr}, rt_addr{rt_addr}, sz{sz}
  {
    assert((word_t)pg_addr % sizeof(word_t) == 0);
    assert(sz % sizeof(word_t) == 0, "%d\r\n", sz);
  }

  void load() const;
  void erase() const;
  void store() const;
  void *operator*() const;
};
