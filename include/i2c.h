#pragma once

#include "types.h"

#define OK 1
#define ERR 0

namespace i2c
{

int probe(uint32_t addr);

void read_bytes(uint32_t addr, uint32_t cmd, byte_t *buf, size_t n);
byte_t read_reg(uint32_t addr, uint32_t cmd);

void write_bytes(uint32_t addr, uint32_t cmd, byte_t *buf, size_t n);
void write_reg(uint32_t addr, uint32_t cmd, uint32_t val);

void init();

} // namespace i2c
