#pragma once

#include "core/types.h"

#define __recover_section__                                                    \
  __attribute__((section(".recover"))) __attribute__((__used__))

inline constexpr size_t N_REC_DATA = 40;
using rec_func_t = void (*)(void *);

using rec_id_t = uint8_t;
inline constexpr rec_id_t null_rec_id = MAX<rec_id_t>;

enum class rec_lev_t {
  NONE,
  RESET,
  BOOT,
};

namespace recover
{
void init();

void set_rec(rec_lev_t rec_lev, const uint8_t *data = nullptr,
             size_t data_sz = 0);

template <typename T>
void set_rec(rec_lev_t rec_lev, const T &data)
{
  set_rec(rec_lev, (const uint8_t *)&data, sizeof(T));
}
} // namespace recover

enum class boot_lev_t {
  FLASH,
  BOOT,
  RESET,
};

namespace boot
{
bool first_boot();
boot_lev_t lev();
void init();
} // namespace boot
