#include "drivers/fram.h"
#include "drivers/i2c.h"

#define FRAM 0x50

namespace fram
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

} // namespace fram

// 15 bit addrs, 8 bits for a page, so 256 pages, 128 bytes long each
// so a block will be 128 bytes
// free blocks will have a header
// used blocks will just be raw bytes
// one levels: small files can be a single block object
// second level: big files can be a block file, listing upto 120 other blocks.
// at the current i2c clock freq of 100Kbps, a block should take about 10ms to
// xfer -- perfect amount of sleep time
