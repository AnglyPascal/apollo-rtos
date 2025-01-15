#pragma once

#include "hardware.h"
#include "types.h"

namespace flash
{

__always_inline__
inline void wait()
{
  while (!NVMC.READY)
    ;
}

void erase(uint32_t *pg_addr);

void write(uint32_t *addr, uint32_t *buffer, size_t sz);

} // namespace flash
