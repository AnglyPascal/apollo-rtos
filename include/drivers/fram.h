#pragma once

#include "core/types.h"

namespace fram
{
using addr_t = uint16_t;

void write(addr_t addr, uint8_t *buf, size_t buf_sz);
void read(addr_t addr, uint8_t *buf, size_t buf_sz);
} // namespace fram
