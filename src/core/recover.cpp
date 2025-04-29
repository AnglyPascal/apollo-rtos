#include "core/recover.h"

#include "core/boot.h"
#include "core/hardware.h"
#include "core/memory.h"
#include "core/sched.h"
#include "core/waitlist.h"
#include "fs/fs.h"
#include "utility/debug.h"

#define REC_MAGIC 0xbabedadd

namespace curr_proc
{
const proc_def_t *def();
uint8_t rec_id();
void set_rec_id(rec_id_t id);
} // namespace curr_proc

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
  uint16_t data_sz;
  lev_t lev;

  void data_reset()
  {
    lev = NONE;
    data_sz = 0;
  }

  bool enabled(lev_t curr_lev) const
  {
    switch (curr_lev) {
    case NONE:
      return false;
    case RESET:
      return lev != NONE;
    default:
      return lev == POWER_OFF;
    }
  }

  void copy_data(const uint8_t *src, size_t sz)
  {
    if (src == nullptr || sz == 0)
      return;

    assert(sz <= N_REC_DATA, TERM);
    memcpy(data, src, sz);
    data_sz = (uint8_t)sz;
  }

  void *copy_data() const
  {
    if (data_sz == 0)
      return nullptr;

    auto ptr = (uint8_t *)kmem::kmalloc(data_sz);
    memcpy(ptr, data, data_sz);
    return ptr;
  }
};

static_assert(alignof(entry_hd_t) == alignof(uint32_t));
static_assert(sizeof(entry_hd_t) ==
              N_REC_DATA + sizeof(uint16_t) + sizeof(lev_t));

template <>
struct alignas(uint32_t) entry_t<PROC> : entry_hd_t {
  const proc_def_t *proc_def;
  _PADDING(4);

  void recover(lev_t curr_lev)
  {
    if (!enabled(curr_lev))
      return;
    sched::reg_proc(proc_def, copy_data());
  }

  void reset()
  {
    proc_def = nullptr;
    data_reset();
  }
};

template <>
struct alignas(uint32_t) entry_t<TASK> : entry_hd_t {
  time_t interval = 0;
  runnable_t task = nullptr;

  void recover(lev_t curr_lev)
  {
    if (!enabled(curr_lev))
      return;

    auto ptr = copy_data();
    if (interval == 0)
      task(ptr);
    else
      waitlist::reg("recover", interval, task, ptr);
  }

  void reset()
  {
    interval = 0;
    task = nullptr;
    data_reset();
  }
};

static_assert(_fram::BLK_SZ % sizeof(entry_t<PROC>) == 0);
static_assert(_fram::BLK_SZ % sizeof(entry_t<TASK>) == 0);

struct tbl_t {
private:
  entry_t<PROC> proc_tbl[N_PROCS] = {};
  entry_t<TASK> task_tbl[N_PROCS] = {};

public:
  template <rec_item_t item>
  entry_t<item> &entry(rec_id_t id)
  {
    if constexpr (item == PROC)
      return proc_tbl[id];
    else
      return task_tbl[id];
  }

  template <rec_item_t item>
  rec_id_t get_rec_id()
  {
    entry_t<item> *tbl;
    if constexpr (item == PROC)
      tbl = proc_tbl;
    else
      tbl = task_tbl;

    for (rec_id_t i = 0; i < N_PROCS; i++)
      if (tbl[i].lev == NONE)
        return i;

    assert(false, H_RESET, "no entry free in %s recovery table\r\n",
           item == PROC ? "proc" : "task");
    return null_rec_id;
  }

  void recover(lev_t curr_lev)
  {
    for (auto &entry : task_tbl) {
      entry.recover(curr_lev);
      entry.reset();
    }

    for (auto &entry : proc_tbl) {
      entry.recover(curr_lev);
      entry.reset();
    }
  }

  void reset()
  {
    for (auto &entry : proc_tbl)
      entry.reset();
    for (auto &entry : task_tbl)
      entry.reset();
  }

  template <rec_item_t item>
  size_t offset(entry_t<item> &addr)
  {
    return (size_t)&addr - (size_t)this;
  }
};

namespace
{
tbl_t tbl __recover_section__ = {};
static_assert(sizeof(tbl) <= 1024);

constexpr fn_t rec_fn = 0;
fram::file_t file;

inline void store_entry(auto &entry)
{
  file.write<SYNC>(&entry, sizeof(entry), tbl.offset(entry));
}
} // namespace

guard_proc::guard_proc(lev_t lev, const uint8_t *data, size_t data_sz)
{
  rec_id = tbl.get_rec_id<PROC>();
  auto &entry = tbl.entry<PROC>(rec_id);
  entry.lev = lev;

  entry.proc_def = curr_proc::def();
  entry.copy_data(data, data_sz);

  store_entry(entry);
}

guard_proc::~guard_proc()
{
  auto &entry = tbl.entry<PROC>(rec_id);
  entry.reset();
  store_entry(entry);
}

guard_task::guard_task(lev_t lev, runnable_t task, time_t interval,
                       const uint8_t *data, size_t data_sz)
{
  rec_id = tbl.get_rec_id<TASK>();
  auto &entry = tbl.entry<TASK>(rec_id);
  entry.lev = lev;

  entry.interval = interval;
  entry.task = task;
  entry.copy_data(data, data_sz);

  store_entry(entry);
}

guard_task::~guard_task()
{
  auto &entry = tbl.entry<TASK>(rec_id);
  entry.reset();
  store_entry(entry);
}

SEC_ADDR(recover);

void init()
{
  auto boot_lev = boot::lev();

  if (boot_lev == boot_lev_t::FLASH)
    fram::remove(rec_fn);

  // needs to be a char file to allow random writes
  fram::open(file, rec_fn, sizeof(tbl), O_WRITE | O_CREATE | O_CHAR_FILE);
  file.mmap(tbl);

  // initialize recovery section
  if (boot_lev != boot_lev_t::RESET)
    SEC_INIT(recover);

  if (boot::is_boot()) {
    tbl.reset();
    file.store();
  }
}

void setup()
{
  auto reset_lev = boot::lev() == boot_lev_t::RESET ? RESET : POWER_OFF;
  tbl.recover(reset_lev);
  file.store();
}

} // namespace recover
