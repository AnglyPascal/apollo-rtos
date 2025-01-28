#pragma once

#include "types.h"

#define __recover_section__                                                    \
  __attribute__((section(".recover"))) __attribute__((__used__))

inline constexpr size_t N_REC_DATA = 48;

enum class rec_lev_t {
  NONE,
  RESET,
  BOOT,
};

using rec_func_t = void (*)(void *);

namespace recovery
{
void *rec_data();

template <typename T>
void store_data(T &src)
{
  *(T *)rec_data() = src;
}

void set_rec_lev(rec_lev_t lev);
void set_rec_lev(rec_lev_t lev, rec_func_t rec_func);

void load();
void store();

} // namespace recovery

void recover();

bool is_first_boot();
