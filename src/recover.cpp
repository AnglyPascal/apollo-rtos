#include "recover.h"
#include "nvm.h"
#include "sched.h"

__extern_C__
uint8_t __recover_pg[];

namespace
{
struct alignas(uint32_t) entry_t {
  rec_func_t rec_func = recovery::def_rec_func;

  rec_lev_t rec_lev = rec_lev_t::NONE;
  uint8_t data[N_REC_DATA];
};

struct recover_table_t {
  uint32_t magic;
  uint32_t n_used;

  entry_t tbl[N_PROCS];
};

static_assert(sizeof(recover_table_t) <= pg_sz);

recover_table_t rec_tbl __recover_section__ = {};
} // namespace

inline constexpr uint32_t magic_value = 0xdeadbeef;

bool is_reset()
{
  return rec_tbl.magic == magic_value;
}

void set_magic()
{
  rec_tbl.magic = magic_value;
}

namespace recovery
{
void *rec_data()
{
  return rec_tbl.tbl[sched::curr_pid()].data;
}

void set_rec_lev(rec_lev_t lev)
{
  set_rec_lev(lev, def_rec_func);
}

void set_rec_lev(rec_lev_t lev, rec_func_t rec_func)
{
  auto &entry = rec_tbl.tbl[sched::curr_pid()];
  entry.rec_lev = lev;
  entry.rec_func = rec_func;
}

void def_rec_func(void *data)
{
  auto proc_def = (proc_def_t *)data;
  sched::reg_proc(proc_def);
}

static uint32_t *rec_tbl_addr = (uint32_t *)__recover_pg;

void store()
{
  nvm_t nvm{rec_tbl_addr, (uint32_t *)&rec_tbl, sizeof(rec_tbl)};
  nvm.store();
}

void load()
{
  nvm_t nvm{rec_tbl_addr, (uint32_t *)&rec_tbl, sizeof(rec_tbl)};
  nvm.load();
}

} // namespace recovery

void recover()
{
  for (auto &[rec_func, rec_lev, data] : rec_tbl.tbl) {
    if (rec_lev != rec_lev_t::NONE) {
      rec_func(data);
      rec_lev = rec_lev_t::NONE;
    }
  }
}

