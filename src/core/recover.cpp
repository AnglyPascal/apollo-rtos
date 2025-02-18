#include "core/recover.h"

#include "core/fs.h"
#include "core/memory.h"
#include "core/sched.h"
#include "utility/debug.h"

#define REC_MAGIC 0xbabedadd

class entry_t
{
  proc_def_t proc_def;
  uint8_t data[N_REC_DATA] = {0xFF};
  // FIXME: incr N_REC_DATA to demonstrate that multi-blk files are not working

public:
  pid_t pid = null_pid;

  bool owned_by(pid_t _pid) const { return (uint8_t)pid == (uint8_t)_pid; }
  bool empty() const { return pid == null_pid; }

  void recover()
  {
    if (!empty())
      sched::reg_proc(&proc_def, data);
    reset();
  }

  void set(pid_t _pid, const proc_def_t *def, const uint8_t *_data, size_t sz)
  {
    pid = _pid;
    proc_def = *def;

    if (_data != nullptr) {
      assert(sz <= N_REC_DATA);
      _memcpy(data, _data, sz);
    }
  }

  void reset() { pid = null_pid; }
};

class recover_table_t
{
private:
  entry_t tbl[N_PROCS] = {{}};

public:
  void recover()
  {
    for (auto &entry : tbl)
      entry.recover();
  }

  void set_entry(pid_t pid, const proc_def_t *def, const uint8_t *data,
                 size_t data_sz)
  {
    size_t empty_idx = 0;
    for (size_t i = 0; i < N_PROCS; i++) {
      auto &entry = tbl[i];

      /* if (entry.owned_by(pid)) { */
      /*   assert(pid == entry.pid); */
      /*   kprintf("%d = %d == %d\r\n", pid == entry.pid, pid, entry.pid); */
      /*   return entry.set(pid, def, data, data_sz); */
      /* } */

      if (entry.empty()) {
        empty_idx = i;
        break;
      }
    }

    tbl[empty_idx].set(pid, def, data, data_sz);
  }

  void reset_entry(pid_t pid)
  {
    for (auto &entry : tbl) {
      if (entry.owned_by(pid))
        return entry.reset();
    }
  }
};

namespace sched
{
void setup_procs(void);
}

namespace curr_proc
{
proc_def_t *def();
}

namespace recover
{
namespace
{
recover_table_t reset_table __recover_section__;

constexpr fn_t recover_fn = 0;
flash::file_t file;
} // namespace

// FIXME: need to set it up so that a recover entry is a resource
// and it needs to be acquired. Because, once killed, a proc's pid can be
// re-used by another pid, and that then overrides the recover entry for that
// previous proc
void init()
{
  auto boot_lev = boot::lev();

  file = flash::open(recover_fn, sizeof(recover_table_t), O_WRITE | O_CREATE);
  flash::mmap(file, reset_table);
  if (boot_lev != boot_lev_t::RESET) {
    reset_table = recover_table_t{};
    flash::store(file);
  }

  // hardware restart if the recover table is corrupt
  if (boot_lev == boot_lev_t::FLASH) {
    sched::setup_procs();
  } else {
    /* reset_table.recover(); */
    /* flash::store(file); */
    sched::setup_procs();
  }
}

void set_rec(rec_lev_t rec_lev, const uint8_t *data, size_t data_sz)
{
  auto pid = curr_proc::pid();

  if (rec_lev == rec_lev_t::NONE)
    reset_table.reset_entry(pid);
  else
    reset_table.set_entry(pid, curr_proc::def(), data, data_sz);

  flash::store(file);
}

/** To be able to remove a process altogether feels too scary
 *  Maybe there should be a way to indicate a process to always run
 *  and also, maybe count "pin reset" as an actual restart, so it should call
 *  setup_procs
 * */

} // namespace recover
