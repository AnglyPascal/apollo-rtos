#pragma once

#include "types.h"

#define OK 0
#define ERR (1 << 3)

namespace i2c
{

int probe(uint8_t addr);

void read_bytes(uint8_t addr, uint8_t cmd, uint8_t *buf, size_t n);
uint8_t read_reg(uint8_t addr, uint8_t cmd);

void write_bytes(uint8_t addr, uint8_t cmd, uint8_t *buf, size_t n);
void write_reg(uint8_t addr, uint8_t cmd, uint8_t val);

void init();

} // namespace i2c
