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

enum class TASK {
  READ,
  WRITE,
};

using TASK::READ;
using TASK::WRITE;

pid_t intr_pid = null_pid;
volatile uint32_t *intr_event = nullptr;

static int wait(volatile uint32_t *event)
{
  intr_pid = sched::curr_pid();
  intr_event = event;
  sched::sleep();
  return I2C0.ERROR ? ERR : OK;
}

static int do_write(byte_t *buf, size_t n)
{
  for (size_t i = 0; i < n; i++) {
    I2C0.TXD = (uint8_t)buf[i];
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

template <TASK task>
inline int xfer(uint8_t addr, byte_t *cmd, size_t cmd_sz, byte_t *buf, size_t n)
{
  auto status = OK;

  I2C0.ADDRESS = addr;

  if (cmd_sz > 0) {
    debug<TRACE>("writing command\r\n");
    I2C0.STARTTX = 1;
    status = do_write(cmd, cmd_sz);
  }

  if constexpr (task == READ) {
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

  if constexpr (task == WRITE) {
    debug<TRACE>("initiating write\r\n");
    if (status == OK && n > 0)
      status = do_write(buf, n);
    stop();
  }

  if (status == OK)
    return OK;

  int error = I2C0.ERRORSRC;
  I2C0.ERRORSRC = I2C_ERRORSRC_All;
  return error;
}

void read_bytes(uint8_t addr, uint8_t cmd, byte_t *buf, size_t n)
{
  xfer<READ>(addr, (byte_t *)&cmd, 1, buf, n);
}

byte_t read_reg(uint8_t addr, uint8_t cmd)
{
  byte_t byte;
  read_bytes(addr, cmd, &byte, 1);
  return byte;
}

void write_bytes(uint8_t addr, uint8_t cmd, byte_t *buf, size_t n)
{
  xfer<WRITE>(addr, (byte_t *)&cmd, 1, buf, n);
}

void write_reg(uint8_t addr, uint8_t cmd, uint8_t val)
{
  write_bytes(addr, cmd, (byte_t *)&val, 1);
}

int probe(uint8_t addr)
{
  char buf = 0;
  return xfer<WRITE>(addr, (byte_t *)&buf, 1, nullptr, 0);
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

inline void which_source()
{
  if (I2C0.STOPPED)
    serial::busy_putc('s');
  else if (I2C0.RXDREADY)
    serial::busy_putc('r');
  else if (I2C0.TXDSENT)
    serial::busy_putc('t');
  else
    serial::busy_putc('e');
}

__extern_C__
void i2c0_spi0_handler(void)
{
  auto irq = I2C0_IRQ;

  assert(intr_pid != null_pid);
  assert(I2C0.ERROR || (intr_event && *intr_event));

  auto pid = intr_pid;
  intr_pid = null_pid;

  *intr_event = 0;
  intr_event = nullptr;

  sched::wakeup(pid);

  clear_pending(irq);
  enable_irq(irq);
}

} // namespace i2c
