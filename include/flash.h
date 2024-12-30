#pragma once

#include "types.h"

namespace flash
{

void erase(uint32_t *pg_addr);

void write(uint32_t *addr, uint32_t *buffer, size_t sz);

} // namespace flash
