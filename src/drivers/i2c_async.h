#pragma once

#include "core/hardware.h"
#include "core/irq.h"
#include "core/sched.h"
#include "drivers/i2c.h"
#include "utility/debug.h"
#include "utility/mutex.h"

namespace i2c
{
namespace async
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

  uint8_t *cmd = nullptr;
  size_t cmd_sz = 0;
  size_t cmd_idx = 0;

  uint8_t *data = nullptr;
  size_t data_sz = 0;
  size_t data_idx = 0;

  fault_t fault = fault_t::NONE;
  uint8_t err_src = I2C_OK;
};

inline volatile state_t state{};

#define clear_event(event)                                                     \
  do {                                                                         \
    assert(event, H_RESET);                                                    \
    event = 0;                                                                 \
  } while (0);

inline chan_t<1> intr_chan;
inline mutex<8> mtx;

void init()
{
  new (&intr_chan) chan_t<1>{};
  new (&mtx) mutex<8>{"i2c"};
}

void handler(void)
{
  intr_guard guard{I2C0_IRQ};

  if (I2C0.ERROR) {
    clear_event(I2C0.ERROR);

    state.fault = fault_t::ERROR;
    state.err_src = (uint8_t)I2C0.ERRORSRC;
    I2C0.ERRORSRC = I2C_ERRORSRC_All;

    state.stage = stage_t::NACK;
    I2C0.STOP = 1;
  }

  else if (state.stage == stage_t::W_CMD) {
    clear_event(I2C0.TXDSENT);

    // still writing the command
    if (state.cmd_idx < state.cmd_sz) {
      auto cmd_idx = state.cmd_idx;
      I2C0.TXD = state.cmd[cmd_idx++];
      state.cmd_idx = cmd_idx;
    }

    // else, finished writing the command

    // no data to send or receive, so stop
    else if (state.data_sz == 0) {
      state.stage = stage_t::NACK;
      I2C0.STOP = 1;
    }

    // if we're writing, then move to TX_DATA state
    else if (state.mode == mode_t::WRITE) {
      state.stage = stage_t::TX_DATA;

      auto data_idx = state.data_idx;
      I2C0.TXD = state.data[data_idx++];
      state.data_idx = data_idx;
    }

    // we're reading, so move to RX_DATA state
    else {
      state.stage = stage_t::RX_DATA;

      if (state.data_idx < state.data_sz - 1)
        I2C0.SHORTS = BIT(I2C_BB_SUSPEND);
      else
        I2C0.SHORTS = BIT(I2C_BB_STOP);

      I2C0.STARTRX = 1;
    }
  }

  else if (state.stage == stage_t::TX_DATA) {
    clear_event(I2C0.TXDSENT);
    if (state.data_idx == state.data_sz) {
      state.stage = stage_t::NACK;
      I2C0.STOP = 1;
    }

    // more bytes to write
    else {
      state.stage = stage_t::TX_DATA;

      auto data_idx = state.data_idx;
      I2C0.TXD = state.data[data_idx++];
      state.data_idx = data_idx;
    }
  }

  else if (state.stage == stage_t::RX_DATA) {
    clear_event(I2C0.RXDREADY);

    auto data_idx = state.data_idx;
    state.data[data_idx++] = (uint8_t)I2C0.RXD;
    state.data_idx = data_idx;

    // finished reading, move to NACK
    if (state.data_idx == state.data_sz) {
      state.stage = stage_t::NACK;
    }

    // continue reading
    else {
      state.stage = stage_t::RX_DATA;

      if (state.data_idx < state.data_sz - 1)
        I2C0.SHORTS = BIT(I2C_BB_SUSPEND);
      else
        I2C0.SHORTS = BIT(I2C_BB_STOP);

      I2C0.RESUME = 1;
    }
  }

  else if (state.stage == stage_t::NACK) {
    clear_event(I2C0.STOPPED);

    if (state.mode == mode_t::READ)
      I2C0.SHORTS = 0;

    state.flag = flag_t::NONE;

    mtx.unlock();

    // then wake up the currently working process
    sched::notify(intr_chan);
  }
}

template <bool is_read>
int xfer(uint8_t addr, uint8_t *cmd, size_t cmd_sz, uint8_t *buf, size_t n)
{
  mtx.lock();

  state.flag = flag_t::BUSY;

  state.mode = is_read ? mode_t::READ : mode_t::WRITE;

  state.data = buf;
  state.data_sz = n;
  state.data_idx = 0;

  state.cmd = cmd + 1;
  state.cmd_sz = cmd_sz - 1;
  state.cmd_idx = 0;

  state.stage = stage_t::W_CMD;

  I2C0.ADDRESS = addr;

  {
    intr_guard guard{I2C0_IRQ};

    I2C0.STARTTX = 1;
    I2C0.TXD = *cmd;
  }

  sched::wait(intr_chan);

  if (state.fault == fault_t::NONE)
    return I2C_OK;

  auto fault = state.fault;
  state.fault = fault_t::NONE;

  if (fault == fault_t::ERROR)
    return state.err_src;

  return I2C_ERR;
}

} // namespace async
} // namespace i2c
