#include "i2c.h"
#include "debug.h"
#include "gpio.h"
#include "hardware.h"
#include "irq.h"
#include "sched.h"

namespace i2c
{

namespace
{

enum class CMD {
  READ,
  WRITE,
};

pid_t intr_pid = null_pid;
volatile uint32_t *intr_event;

static int wait(volatile uint32_t *event)
{
  intr_pid = sched::curr_pid();
  intr_event = event;
  sched::sleep();

  if (I2C0.ERROR) {
    return ERR;
  }

  return OK;
}

static int do_write(byte_t *buf, size_t n)
{
  for (size_t i = 0; i < n; i++) {
    I2C0.TXD = (uint32_t)buf[i];
    auto status = wait(&I2C0.TXDSENT);
    if (status != OK)
      return status;
  }
  return OK;
}

static void stop()
{
  I2C0.STOP = 1;
  wait(&I2C0.STOPPED);
}

} // namespace

template <CMD task>
inline int xfer(uint32_t addr, byte_t *cmd, size_t cmd_sz, byte_t *buf,
                size_t n)
{
  auto status = OK;
  auto error = 0;

  I2C0.ADDRESS = addr;

  if (cmd_sz > 0) {
    debug<TRACE>("writing command\r\n");
    I2C0.STARTTX = 1;
    status = do_write(cmd, cmd_sz);
  }

  if constexpr (task == CMD::READ) {
    debug<TRACE>("initiating read\r\n");
    for (size_t i = 0; i < n; i++) {
      /* On all but the last byte, use SUSPEND to send an ACK after receiving
       * the byte. Use STOP to send a NACK at the end. */
      if (i < n - 1)
        I2C0.SHORTS = BIT(I2C_BB_SUSPEND);
      else
        I2C0.SHORTS = BIT(I2C_BB_STOP);

      /* Start the first byte with STARTTX,
       * and the rest with RESUME following the SUSPEND. */
      if (i == 0)
        I2C0.STARTRX = 1;
      else
        I2C0.RESUME = 1;

      status = wait(&I2C0.RXDREADY);
      if (status != OK)
        break;

      buf[i] = (byte_t)I2C0.RXD;
    }

    if (status == OK) {
      wait(&I2C0.STOPPED);
    } else {
      stop();
    }

    I2C0.SHORTS = 0;
  }

  if constexpr (task == CMD::WRITE) {
    debug<TRACE>("initiating write\r\n");
    if (status == OK && n > 0)
      status = do_write(buf, n);
    stop();
  }

  if (status != OK) {
    error = I2C0.ERRORSRC;
    I2C0.ERRORSRC = I2C_ERRORSRC_All;
  }

  if (error)
    return ERR;

  return status;
}

void read_bytes(uint32_t addr, uint32_t cmd, byte_t *buf, size_t n)
{
  xfer<CMD::READ>(addr, (byte_t *)&cmd, 1, buf, n);
}

byte_t read_reg(uint32_t addr, uint32_t cmd)
{
  byte_t byte;
  read_bytes(addr, cmd, &byte, 1);
  return byte;
}

void write_bytes(uint32_t addr, uint32_t cmd, byte_t *buf, size_t n)
{
  xfer<CMD::WRITE>(addr, (byte_t *)&cmd, 1, buf, n);
}

void write_reg(uint32_t addr, uint32_t cmd, uint32_t val)
{
  write_bytes(addr, cmd, (byte_t *)&val, 1);
}

int probe(uint32_t addr)
{
  char buf = 0;
  return xfer<CMD::WRITE>(addr, (byte_t *)&buf, 1, nullptr, 0);
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
}

__extern_C__
void i2c0_spi0_handler(void)
{
  auto irq = I2C0_IRQ;

  if (I2C0.ERROR || *intr_event) {
    auto pid = intr_pid;
    intr_pid = null_pid;

    *intr_event = 0;
    intr_event = nullptr;

    sched::wakeup(pid);
  }

  clear_pending(irq);
  enable_irq(irq);
}

} // namespace i2c
