#pragma once

#include "core/hardware.h"
#include "core/irq.h"
#include "core/sched.h"
#include "drivers/i2c.h"
#include "utility/debug.h"

namespace i2c_non_blocking
{

enum class stage_t {
  NONE,

  W_CMD,

  TX_DATA,
  RX_DATA,

  NACK,
};

enum class mode_t {
  READ,
  WRITE,
};

enum class flag_t {
  NONE,
  BUSY,
};

enum class fault_t {
  NONE,
  BUSY,
  TIMEOUT,
  ERROR,
};

struct state_t {
  stage_t stage = stage_t::NONE;
  mode_t mode = mode_t::WRITE;
  flag_t flag = flag_t::NONE;

  uint8_t dev_addr = 0;
  uint8_t cmd = 0;

  uint8_t *data = nullptr;
  size_t data_sz = 0;
  size_t data_idx = 0;

  fault_t fault = fault_t::NONE;
  uint8_t err_src = I2C_OK;
};

inline state_t state{};

__always_inline__
inline void clear_event(volatile uint32_t &event)
{
  assert(event);
  event = 0;
}

inline chan_t intr_chan;

template <typename T = void>
void handler(void)
{
  auto irq = I2C0_IRQ;
  disable_irq(irq);

  if (I2C0.ERROR) {
    clear_event(I2C0.ERROR);

    state.fault = fault_t::ERROR;
    state.err_src = (uint8_t)I2C0.ERRORSRC;
    I2C0.ERRORSRC = I2C_ERRORSRC_All;

    state.stage = stage_t::NACK;
    I2C0.STOP = 1;

    goto clear_intr;
  }

  if (state.stage == stage_t::W_CMD) {
    clear_event(I2C0.TXDSENT);

    if (state.data_sz == 0) {
      state.stage = stage_t::NACK;
      I2C0.STOP = 1;
    } else {
      if (state.mode == mode_t::WRITE) {
        state.stage = stage_t::TX_DATA;
        I2C0.TXD = state.data[state.data_idx++];
      } else {
        state.stage = stage_t::RX_DATA;

        if (state.data_idx < state.data_sz - 1)
          I2C0.SHORTS = BIT(I2C_BB_SUSPEND);
        else
          I2C0.SHORTS = BIT(I2C_BB_STOP);

        I2C0.STARTRX = 1;
      }
    }

    goto clear_intr;
  }

  if (state.stage == stage_t::TX_DATA) {
    clear_event(I2C0.TXDSENT);
    if (state.data_idx == state.data_sz) {
      state.stage = stage_t::NACK;
      I2C0.STOP = 1;
    } else {
      state.stage = stage_t::TX_DATA;
      I2C0.TXD = state.data[state.data_idx++];
    }

    goto clear_intr;
  }

  if (state.stage == stage_t::RX_DATA) {
    clear_event(I2C0.RXDREADY);

    state.data[state.data_idx++] = (uint8_t)I2C0.RXD;

    if (state.data_idx == state.data_sz) {
      state.stage = stage_t::NACK;
    } else {
      state.stage = stage_t::RX_DATA;

      if (state.data_idx < state.data_sz - 1)
        I2C0.SHORTS = BIT(I2C_BB_SUSPEND);
      else
        I2C0.SHORTS = BIT(I2C_BB_STOP);

      I2C0.RESUME = 1;
    }

    goto clear_intr;
  }

  if (state.stage == stage_t::NACK) {
    clear_event(I2C0.STOPPED);

    if (state.mode == mode_t::READ)
      I2C0.SHORTS = 0;

    state.flag = flag_t::NONE;
    sched::notify(&intr_chan);

    goto clear_intr;
  }

clear_intr:
  clear_pending(irq);
  enable_irq(irq);
}

template <bool is_read>
int xfer(uint8_t addr, uint8_t *cmd, size_t cmd_sz, uint8_t *buf, size_t n)
{
  if (state.flag == flag_t::BUSY) {
    state.fault = fault_t::BUSY;
    return I2C_BUSY;
  }
  state.flag = flag_t::BUSY;

  state.mode = is_read ? mode_t::READ : mode_t::WRITE;

  state.stage = stage_t::W_CMD;
  state.data = buf;
  state.data_sz = n;
  state.data_idx = 0;

  I2C0.ADDRESS = addr;

  intr_disable();

  I2C0.STARTTX = 1;
  I2C0.TXD = *cmd;

  sched::wait(&intr_chan);

  if (state.fault == fault_t::NONE)
    return I2C_OK;

  auto fault = state.fault;
  state.fault = fault_t::NONE;

  if (fault == fault_t::ERROR)
    return state.err_src;

  return I2C_ERR;
}

} // namespace i2c_non_blocking
