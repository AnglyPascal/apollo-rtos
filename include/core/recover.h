#pragma once

#include "core/types.h"

#define __recover_section__                                                    \
  __attribute__((section(".recover"))) __attribute__((__used__))

inline constexpr size_t N_REC_DATA = 19;
using rec_func_t = void (*)(void *);

using rec_id_t = uint8_t;
inline constexpr rec_id_t null_rec_id = MAX<rec_id_t>;

enum lev_t : uint8_t {
  NONE = 0,
  RESET = 1,
  POWER_OFF = 2,
};

namespace recover
{
void init();

class guard_proc
{
private:
  rec_id_t rec_id;

public:
  guard_proc(lev_t lev, const uint8_t *data = nullptr, size_t data_sz = 0);

  template <typename T>
  guard_proc(lev_t lev, const T &data)
      : guard_proc(lev, (const uint8_t *)&data, sizeof(T))
  {
  }

  ~guard_proc();
};

class guard_task
{
private:
  rec_id_t rec_id;

public:
  guard_task(lev_t lev, runnable_t task, time_t interval = 0,
             const uint8_t *data = nullptr, size_t data_sz = 0);

  template <typename T>
  guard_task(lev_t lev, runnable_t task, time_t interval, const T &data)
      : guard_task(lev, task, interval, (const uint8_t *)&data, sizeof(T))
  {
  }

  ~guard_task();
};

} // namespace recover
