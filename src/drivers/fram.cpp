#include "fs/fs.h"

#include "drivers/i2c.h"

#define FRAM 0x50

namespace _fram
{
void write(addr_t addr, uint8_t *buf, size_t buf_sz)
{
  uint8_t cmd[] = {(uint8_t)(addr >> 8), (uint8_t)addr};
  i2c::write_bytes(FRAM, cmd, 2, buf, buf_sz);
}

void read(addr_t addr, uint8_t *buf, size_t buf_sz)
{
  uint8_t cmd[] = {(uint8_t)(addr >> 8), (uint8_t)addr};
  i2c::read_bytes(FRAM, cmd, 2, buf, buf_sz);
}

} // namespace _fram

