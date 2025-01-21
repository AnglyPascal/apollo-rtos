#include "recover.h"
#include "sched.h"

namespace
{
struct alignas(uint32_t) entry_t {
  bool in_use = false;
  runnable_t recover_func = nullptr;
  void *param = nullptr;
};

struct recover_table_t {
  uint32_t magic;
  uint32_t n_used;

  entry_t tbl[N_PROCS];
};

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
void alloc(runnable_t recover_func, void *param)
{
  auto pid = sched::curr_pid();
  rec_tbl.tbl[pid] = {true, recover_func, param};
}

void dealloc()
{
  auto pid = sched::curr_pid();
  rec_tbl.tbl[pid] = {};
}
} // namespace recovery

void recover()
{
  for (auto &entry : rec_tbl.tbl) {
    auto [in_use, rec_func, param] = entry;
    if (in_use) {
      rec_func(param);
    }
    entry = {};
  }
}

