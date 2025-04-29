#include "drivers/i2c.h"

#include "core/hardware.h"
#include "core/irq.h"
#include "core/sched.h"
#include "drivers/gpio.h"
#include "utility/debug.h"
#include "utility/profile.h"

#include "i2c_async.h"
#include "i2c_sync.h"

namespace i2c
{

enum : bool {
  READ = true,
  WRITE = false,
};

int read_bytes(uint8_t addr, uint8_t *cmd, size_t cmd_sz, uint8_t *buf,
               size_t n, bool sync)
{
  PROFILE_THIS(0);
  if (sync || !curr_proc::set_up())
    return sync::xfer<READ>(addr, cmd, cmd_sz, buf, n);
  return async::xfer<READ>(addr, cmd, cmd_sz, buf, n);
}

uint8_t read_reg(uint8_t addr, uint8_t *cmd, size_t cmd_sz)
{
  uint8_t byte{};
  read_bytes(addr, cmd, cmd_sz, &byte, 1);
  return byte;
}

int write_bytes(uint8_t addr, uint8_t *cmd, size_t cmd_sz, uint8_t *buf,
                size_t n, bool sync)
{
  PROFILE_THIS(1);
  if (sync || !curr_proc::set_up())
    return sync::xfer<WRITE>(addr, cmd, cmd_sz, buf, n);
  return async::xfer<WRITE>(addr, cmd, cmd_sz, buf, n);
}

void write_reg(uint8_t addr, uint8_t *cmd, size_t cmd_sz, uint8_t val)
{
  write_bytes(addr, cmd, cmd_sz, (uint8_t *)&val, 1);
}

int probe(uint8_t addr)
{
  uint8_t buf = 0;
  return sync::xfer<WRITE>(addr, &buf, 1, nullptr, 0);
}

void init()
{
  /* Configure pins -- thanks to friends at University of Cantabria */
  gpio::drive(I2C0_SCL, GPIO_DRIVE_S0D1);
  gpio::drive(I2C0_SDA, GPIO_DRIVE_S0D1);

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

  async::init();

  /* scan(); */
}

void scan()
{
  kprintf("i2c devices:\r\n");
  int i = 0;
  for (uint8_t addr = 0x03; addr <= 0x77; addr++)
    if (probe(addr) == I2C_OK)
      kprintf("| %d: %x\r\n", i++, addr);
}

__extern_C__ void i2c0_spi0_handler(void) { async::handler(); }
} // namespace i2c
