#include "fs/fs.h"

#include "drivers/i2c.h"

#define FRAM 0x50

// FIXME: calculate and return how many bytes were properly transfered
namespace _fram
{
size_t write(addr_t addr, uint8_t *buf, size_t buf_sz)
{
  uint8_t cmd[] = {(uint8_t)(addr >> 8), (uint8_t)addr};
  i2c::write_bytes(FRAM, cmd, 2, buf, buf_sz);
  return buf_sz;
}

size_t write_sync(addr_t addr, uint8_t *buf, size_t buf_sz)
{
  uint8_t cmd[] = {(uint8_t)(addr >> 8), (uint8_t)addr};
  i2c::write_bytes(FRAM, cmd, 2, buf, buf_sz, true);
  return buf_sz;
}

size_t read(addr_t addr, uint8_t *buf, size_t buf_sz)
{
  uint8_t cmd[] = {(uint8_t)(addr >> 8), (uint8_t)addr};
  i2c::read_bytes(FRAM, cmd, 2, buf, buf_sz);
  return buf_sz;
}

size_t read_sync(addr_t addr, uint8_t *buf, size_t buf_sz)
{
  uint8_t cmd[] = {(uint8_t)(addr >> 8), (uint8_t)addr};
  i2c::read_bytes(FRAM, cmd, 2, buf, buf_sz, true);
  return buf_sz;
}

} // namespace _fram

