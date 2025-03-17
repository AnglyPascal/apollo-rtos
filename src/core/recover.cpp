#include "core/recover.h"

#include "core/boot.h"
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
  uint8_t data_sz;
  lev_t lev = NONE;

  void reset()
  {
    lev = NONE;
    data_sz = 0;
  }

  void copy_data(const uint8_t *src, size_t sz)
  {
    if (src == nullptr || sz == 0)
      return;

    assert(sz <= N_REC_DATA, TERM);
    _memcpy(data, src, sz);
    data_sz = (uint8_t)sz;
  }

  void *copy_data() const
  {
    if (data_sz == 0)
      return nullptr;

    auto ptr = (uint8_t *)kmem::kmalloc(data_sz);
    _memcpy(ptr, data, data_sz);
    return ptr;
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
    sched::reg_proc(&proc_def, copy_data());
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

    auto ptr = copy_data();
    if (interval == 0)
      task(ptr);
    else
      waitlist::reg("recover", interval, task, ptr);
  }
};

struct entry_pair_t : std::pair<entry_t<PROC>, entry_t<TASK>> {
  template <rec_item_t item>
  entry_t<item> &get()
  {
    if constexpr (item == PROC)
      return first;
    else
      return second;
  }

  void recover(lev_t curr_lev)
  {
    first.recover(curr_lev);
    second.recover(curr_lev);
  }

  void reset()
  {
    first.reset();
    second.reset();
  }
};

static_assert(_fram::BLK_SZ % sizeof(entry_pair_t) == 0);

struct tbl_t {
private:
  entry_pair_t tbl[N_PROCS] = {};

public:
  template <rec_item_t item>
  entry_t<item> &entry(rec_id_t id)
  {
    return tbl[id].get<item>();
  }

  template <rec_item_t item>
  rec_id_t get_rec_id()
  {
    for (rec_id_t i = 0; i < N_PROCS; i++) {
      if (tbl[i].get<item>().lev == NONE)
        return i;
    }
    assert(false, H_RESET, "no recover table entry free\r\n");
    return null_rec_id;
  }

  void recover(lev_t curr_lev)
  {
    for (auto &entry_pair : tbl) {
      entry_pair.recover(curr_lev);
      entry_pair.reset();
    }
  }

  void reset()
  {
    for (auto &entry : tbl)
      entry.reset();
  }

  size_t offset(void *addr) { return (size_t)addr - (size_t)this; }
};

namespace
{
tbl_t tbl __recover_section__ = {};
static_assert(sizeof(tbl) <= 1024);

constexpr fn_t rec_fn = 0;
fram::file_t file;
} // namespace

guard_proc::guard_proc(lev_t lev, const uint8_t *data, size_t data_sz)
{
  rec_id = tbl.get_rec_id<PROC>();
  auto &entry = tbl.entry<PROC>(rec_id);
  entry.lev = lev;

  entry.proc_def = *curr_proc::def();
  entry.copy_data(data, data_sz);

  file.store(tbl.offset(&entry), sizeof(entry));
}

guard_proc::~guard_proc()
{
  auto &entry = tbl.entry<PROC>(rec_id);
  entry.reset();
  file.store(tbl.offset(&entry), sizeof(entry));
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

  file.store(tbl.offset(&entry), sizeof(entry));
}

guard_task::~guard_task()
{
  auto &entry = tbl.entry<TASK>(rec_id);
  entry.reset();
  file.store(tbl.offset(&entry), sizeof(entry));
}

void init()
{
  fram::open(file, rec_fn, sizeof(tbl), O_WRITE | O_CREATE | O_PERM);
  file.mmap(tbl);

  auto boot_lev = boot::lev();
  if (boot_lev == boot_lev_t::BOOT || boot_lev == boot_lev_t::FLASH) {
    tbl.reset();
    sched::setup_procs();
  } else {
    auto reset_lev = boot_lev == boot_lev_t::RESET ? RESET : POWER_OFF;
    tbl.recover(reset_lev);
  }

  file.store();
}

} // namespace recover
