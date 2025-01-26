#include "recover.h"
#include "nvm.h"
#include "sched.h"

__extern_C__
uint8_t __recover_pg[];

#define REC_MAGIC 0xbabedadd

namespace
{
struct alignas(word_t) entry_t {
  uint32_t magic;
  rec_func_t rec_func = nullptr;

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
bool first_boot = false;

} // namespace

void set_boot()
{
  first_boot = true;
  rec_tbl.magic = REC_MAGIC;
}

bool is_first_boot() { return first_boot; }
bool is_reset() { return rec_tbl.magic == REC_MAGIC; }

namespace recovery
{
void *rec_data() { return rec_tbl.tbl[sched::curr_pid()].data; }

inline void def_rec_func(void *data)
{
  auto proc_def = (proc_def_t *)data;
  sched::reg_proc(proc_def);
}

void set_rec_lev(rec_lev_t lev) { set_rec_lev(lev, def_rec_func); }

void set_rec_lev(rec_lev_t lev, rec_func_t rec_func)
{
  auto &entry = rec_tbl.tbl[sched::curr_pid()];
  entry.magic = REC_MAGIC;
  entry.rec_lev = lev;
  entry.rec_func = rec_func;
}

static word_t *rec_tbl_addr = (word_t *)__recover_pg;

void store()
{
  nvm_t nvm{rec_tbl_addr, (word_t *)&rec_tbl, sizeof(rec_tbl)};
  nvm.store();
}

void load()
{
  nvm_t nvm{rec_tbl_addr, (word_t *)&rec_tbl, sizeof(rec_tbl)};
  nvm.load();
}

} // namespace recovery

void recover()
{
  for (auto &[magic, rec_func, rec_lev, data] : rec_tbl.tbl) {
    if (magic == REC_MAGIC && rec_lev != rec_lev_t::NONE) {
      rec_func(data);
      rec_lev = rec_lev_t::NONE;
    }
  }
}

