#include "core/recover.h"

#include "core/memory.h"
#include "core/sched.h"
#include "core/waitlist.h"
#include "fs/fs.h"
#include "utility/debug.h"

#define REC_MAGIC 0xbabedadd

namespace curr_proc
{
proc_def_t *def();
uint8_t rec_id();
void set_rec_id(rec_id_t id);
} // namespace curr_proc

namespace sched
{
void setup_procs(void);
}

namespace recover
{
enum rec_item_t {
  PROC,
  TASK,
};

template <rec_item_t item>
struct entry_t;

struct alignas(uint32_t) entry_hd_t {
  uint8_t data[N_REC_DATA] = {0xFF};
  lev_t lev = NONE;

  void reset()
  {
    lev = NONE;
    _memset(data, 0xFF, N_REC_DATA);
  }

  void copy_data(const uint8_t *src, size_t sz)
  {
    if (src == nullptr)
      return;
    assert(sz <= N_REC_DATA);
    _memcpy(data, src, sz);
  }
};

static_assert(sizeof(entry_hd_t) % sizeof(uint32_t) == 0);
static_assert(alignof(entry_hd_t) == alignof(uint32_t));

template <>
struct alignas(uint32_t) entry_t<PROC> : entry_hd_t {
  proc_def_t proc_def;

  void recover(lev_t curr_lev)
  {
    if (lev < curr_lev)
      return;
    sched::reg_proc(&proc_def, data);
  }
};

template <>
struct alignas(uint32_t) entry_t<TASK> : entry_hd_t {
  time_t interval = 0;
  runnable_t task = nullptr;

  void recover(lev_t curr_lev)
  {
    if (lev < curr_lev)
      return;

    if (interval == 0)
      task(data);
    else
      waitlist::reg("recover", interval, task, data);
  }
};

template <rec_item_t item>
struct tbl_t {
private:
  entry_t<item> tbl[N_PROCS] = {};

public:
  entry_t<item> &operator[](rec_id_t id) { return tbl[id]; }

  rec_id_t get_rec_id()
  {
    for (rec_id_t i = 0; i < N_PROCS; i++) {
      if (tbl[i].lev == NONE)
        return i;
    }
    assert(false, "no recover table entry free\r\n");
    return null_rec_id;
  }

  void recover(lev_t curr_lev)
  {
    for (auto &entry : tbl) {
      entry.recover(curr_lev);
      entry.reset();
    }
  }

  void reset()
  {
    for (auto &entry : tbl)
      entry.reset();
  }
};

struct rec_tbl_t {
  tbl_t<PROC> proc_tbl;
  tbl_t<TASK> task_tbl;

  void reset()
  {
    proc_tbl.reset();
    task_tbl.reset();
  }

  void recover(lev_t curr_lev)
  {
    proc_tbl.recover(curr_lev);
    task_tbl.recover(curr_lev);
  }
};
static_assert(sizeof(rec_tbl_t) <= 1024);

namespace
{
rec_tbl_t rec_tbl __recover_section__ = {};
constexpr fn_t rec_fn = 0;
fram::file_t file;
} // namespace

guard_proc::guard_proc(lev_t lev, const uint8_t *data, size_t data_sz)
{
  auto &tbl = rec_tbl.proc_tbl;
  rec_id = tbl.get_rec_id();

  auto &entry = tbl[rec_id];
  entry.lev = lev;

  entry.proc_def = *curr_proc::def();
  entry.copy_data(data, data_sz);

  file.store();
}

guard_proc::~guard_proc()
{
  auto &entry = rec_tbl.proc_tbl[rec_id];
  entry.reset();
  file.store();
}

guard_task::guard_task(lev_t lev, runnable_t task, time_t interval,
                       const uint8_t *data, size_t data_sz)
{
  auto &tbl = rec_tbl.task_tbl;
  rec_id = tbl.get_rec_id();

  auto &entry = tbl[rec_id];
  entry.lev = lev;

  entry.interval = interval;
  entry.task = task;
  entry.copy_data(data, data_sz);

  file.store();
}

guard_task::~guard_task()
{
  auto &entry = rec_tbl.task_tbl[rec_id];
  entry.reset();
  file.store();
}

void init()
{
  // FIXME: this reuses the previous version of the file
  // so any changes to the file size will cause conflicts
  fram::open(file, rec_fn, sizeof(rec_tbl_t), O_WRITE | O_CREATE | O_PERM);
  file.mmap(rec_tbl);

  auto boot_lev = boot::lev();
  if (boot_lev == boot_lev_t::BOOT || boot_lev == boot_lev_t::FLASH) {
    rec_tbl.reset();
    sched::setup_procs();
  } else {
    auto reset_lev = boot_lev == boot_lev_t::RESET ? RESET : POWER_OFF;
    rec_tbl.recover(reset_lev);
  }

  file.store();
}

} // namespace recover
