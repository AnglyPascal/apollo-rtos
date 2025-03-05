#include "core/recover.h"

#include "core/fs.h"
#include "core/memory.h"
#include "core/sched.h"
#include "core/waitlist.h"
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

template <>
struct entry_t<PROC> {
  lev_t lev = NONE;
  proc_def_t proc_def;
  uint8_t data[N_REC_DATA] = {0xFF};

  void reset()
  {
    lev = NONE;
    _memset(data, 0xFF, N_REC_DATA);
  }
};

template <>
struct entry_t<TASK> {
  lev_t lev = NONE;
  time_t interval = 0;
  runnable_t task = nullptr;
  uint8_t data[N_REC_DATA] = {0xFF};

  void reset()
  {
    lev = NONE;
    _memset(data, 0xFF, N_REC_DATA);
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

  auto begin() { return tbl; }
  auto end() { return tbl + N_PROCS; }
};

struct rec_tbl_t {
  tbl_t<PROC> proc_tbl;
  tbl_t<TASK> task_tbl;

  void reset()
  {
    for (auto &entry : proc_tbl)
      entry.reset();
    for (auto &entry : task_tbl)
      entry.reset();
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

  if (data != nullptr) {
    assert(data_sz <= N_REC_DATA);
    _memcpy(entry.data, data, data_sz);
  }

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

  if (data != nullptr) {
    assert(data_sz <= N_REC_DATA);
    _memcpy(entry.data, data, data_sz);
  }

  file.store();
}

guard_task::~guard_task()
{
  auto &entry = rec_tbl.task_tbl[rec_id];
  entry.reset();
  file.store();
}

inline void recover()
{
  auto &task_tbl = rec_tbl.task_tbl;
  for (auto &entry : task_tbl) {
    if (entry.lev != NONE && entry.task != nullptr) {
      if (entry.interval == 0)
        entry.task(entry.data);
      else
        waitlist::reg("recover", entry.interval, entry.task, entry.data);
    }
    entry.reset();
  }

  auto &proc_tbl = rec_tbl.proc_tbl;
  for (auto &entry : proc_tbl) {
    if (entry.lev != NONE)
      sched::reg_proc(&entry.proc_def, entry.data);
    entry.reset();
  }

  file.store();
}

void init()
{
  fram::open(file, rec_fn, sizeof(rec_tbl_t), O_WRITE | O_CREATE | O_PERM);
  file.mmap(rec_tbl);

  // TODO: hardware restart if the recover table is corrupt
  if (boot::lev() != boot_lev_t::RESET) {
    rec_tbl.reset();
    sched::setup_procs();
  } else {
    recover();
  }

  file.store();
}

} // namespace recover
