#pragma once

#include "core/types.h"

#define I2C_OK 0
#define I2C_OVERRUN (1 << 0)
#define I2C_ANACK (1 << 1)
#define I2C_DNACK (1 << 2)
#define I2C_ERR (1 << 3)
#define I2C_BUSY (1 << 4)

namespace i2c
{

int probe(uint8_t addr);

void read_bytes(uint8_t addr, uint8_t *cmd, size_t cmd_sz, uint8_t *buf,
                size_t n);
uint8_t read_reg(uint8_t addr, uint8_t *cmd, size_t cmd_sz);

void write_bytes(uint8_t addr, uint8_t *cmd, size_t cmd_sz, uint8_t *buf,
                 size_t n);
void write_reg(uint8_t addr, uint8_t *cmd, size_t cmd_sz, uint8_t val);

void init();
void irq_en();

void scan();
} // namespace i2c
