#pragma once

#include "types.h"

class nvm_t
{
  uint32_t *nvm_addr;
  uint32_t *ram_addr;
  size_t len;

public:
  nvm_t(uint32_t *addr, size_t len);
  nvm_t(size_t len);

  void load();
  void store();

  void *operator*();

  ~nvm_t();
};
