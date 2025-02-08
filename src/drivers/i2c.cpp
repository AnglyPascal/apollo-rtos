#include "drivers/i2c.h"
#include "core/hardware.h"
#include "core/irq.h"
#include "core/sched.h"
#include "drivers/gpio.h"
#include "utility/circular_buffer.h"
#include "utility/debug.h"

#include "i2c_blocking.h"
#include "i2c_non_blocking.h"

namespace i2c
{
using namespace i2c_non_blocking;

void read_bytes(uint8_t addr, uint8_t cmd, uint8_t *buf, size_t n)
{
  xfer<true>(addr, (uint8_t *)&cmd, 1, buf, n);
}

uint8_t read_reg(uint8_t addr, uint8_t cmd)
{
  uint8_t byte{};
  read_bytes(addr, cmd, &byte, 1);
  return byte;
}

void write_bytes(uint8_t addr, uint8_t cmd, uint8_t *buf, size_t n)
{
  xfer<false>(addr, (uint8_t *)&cmd, 1, buf, n);
}

void write_reg(uint8_t addr, uint8_t cmd, uint8_t val)
{
  write_bytes(addr, cmd, (uint8_t *)&val, 1);
}

int probe(uint8_t addr)
{
  char buf = 0;
  return xfer<false>(addr, (uint8_t *)&buf, 1, nullptr, 0);
}

void init()
{
  /* Configure pins -- thanks to friends at University of Cantabria */
  gpio_drive(I2C0_SCL, GPIO_DRIVE_S0D1);
  gpio_drive(I2C0_SDA, GPIO_DRIVE_S0D1);

  /* Configure I2C hardware */
  I2C0.PSELSCL = I2C0_SCL;
  I2C0.PSELSDA = I2C0_SDA;
  I2C0.FREQUENCY = I2C_FREQUENCY_100kHz;
  I2C0.ENABLE = I2C_ENABLE_Enabled;

  /* Enable interrupts */
  I2C0.INTEN = BIT(I2C_INT_RXDREADY) | BIT(I2C_INT_TXDSENT) |
               BIT(I2C_INT_STOPPED) | BIT(I2C_INT_ERROR);
  I2C0.INTENCLR = BIT(I2C_INT_BB);

  enable_irq(I2C0_IRQ);
  irq_priority(I2C0_IRQ, IRQ_PRIO_M1);
}

__extern_C__
void i2c0_spi0_handler(void)
{
  handler();
}
} // namespace i2c
