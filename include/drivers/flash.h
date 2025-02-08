#pragma once

#include "core/memory.h"
#include "core/types.h"
#include "utility/debug.h"

// runtime representation of a nvm page
class nvm_t
{
protected:
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

using page_guard_t = uint32_t;

class pg_t : nvm_t
{
public:
  pg_t() : nvm_t{} {}

  pg_t(word_t *pg_addr, word_t *rt_addr, size_t sz)
      : nvm_t{pg_addr, rt_addr, sz}
  {
    assert((uint32_t)pg_addr % pg_sz == 0);
  }

  void load() const;
  void store() const;
  using nvm_t::erase;

  bool is_valid() const;
};
