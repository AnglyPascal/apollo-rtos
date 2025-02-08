#pragma once

#include "core/hardware.h"
#include "core/irq.h"
#include "core/sched.h"
#include "drivers/i2c.h"
#include "utility/debug.h"

namespace i2c_blocking
{

namespace
{
template <typename T = void>
int wait(volatile uint32_t *event)
{
  while (!*event)
    ;
  return I2C0.ERROR ? I2C0.ERRORSRC : I2C_OK;
}

inline int do_write(uint8_t *buf, size_t n)
{
  for (size_t i = 0; i < n; i++) {
    I2C0.TXD = buf[i];
    auto status = wait(&I2C0.TXDSENT);
    if (status != I2C_OK)
      return status;
  }
  return I2C_OK;
}

inline void stop()
{
  I2C0.STOP = 1;
  wait(&I2C0.STOPPED);
}
} // namespace

template <bool is_read>
int xfer(uint8_t addr, uint8_t *cmd, size_t cmd_sz, uint8_t *buf, size_t n)
{
  auto status = I2C_OK;

  I2C0.ADDRESS = addr;

  if (cmd_sz > 0) {
    debug<TRACE>("writing command\r\n");
    I2C0.STARTTX = 1;
    status = do_write(cmd, cmd_sz);
  }

  if constexpr (is_read) {
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
      if (status != I2C_OK)
        break;

      buf[i] = (uint8_t)I2C0.RXD;
    }

    if (status == I2C_OK) {
      wait(&I2C0.STOPPED);
    } else {
      stop();
    }

    I2C0.SHORTS = 0;
  }

  if constexpr (!is_read) {
    debug<TRACE>("initiating write\r\n");
    if (status == I2C_OK && n > 0)
      status = do_write(buf, n);
    stop();
  }

  if (status == I2C_OK)
    return I2C_OK;

  int error = I2C0.ERRORSRC;
  I2C0.ERRORSRC = I2C_ERRORSRC_All;
  return error;
}

template <typename T = void>
void handler(void)
{
  auto irq = I2C0_IRQ;
  clear_pending(irq);
  enable_irq(irq);
}

} // namespace i2c_blocking
