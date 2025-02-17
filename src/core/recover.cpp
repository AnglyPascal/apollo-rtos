#include "core/recover.h"

#include "core/fs.h"
#include "core/memory.h"
#include "core/sched.h"
#include "utility/debug.h"

#define REC_MAGIC 0xbabedadd

__extern_C__
byte_t __recover_load[],
    __recover_start[], __recover_end[];

namespace fs
{
bool first_boot();
}

namespace boot
{
namespace
{
bool power_on = false;
uint32_t power_magic __recover_section__ = REC_MAGIC;
} // namespace

void init()
{
  power_on = power_magic != REC_MAGIC;
  power_magic = REC_MAGIC;

  auto _lev = lev();
  if (_lev == boot_lev_t::RESET)
    kprintf("reset\r\n");
  else if (_lev == boot_lev_t::FLASH)
    kprintf("flash\r\n");
  else
    kprintf("boot\r\n");

  // initialize recovery section
  if (boot::lev() != boot_lev_t::RESET)
    _memcpy(__recover_start, __recover_load, __recover_end - __recover_start);
}

boot_lev_t lev()
{
  if (fs::first_boot())
    return boot_lev_t::FLASH;
  return power_on ? boot_lev_t::BOOT : boot_lev_t::RESET;
}
} // namespace boot

class entry_t
{
  bool valid = false;
  proc_def_t proc_def;
  uint8_t data[N_REC_DATA] = {0xFF};
  // FIXME: try increasing N_REC_DATA to demonstrate that multi-blk files are
  // not working

public:
  void recover()
  {
    if (valid)
      sched::reg_proc(&proc_def, data);
    valid = false;
  }

  void set(const proc_def_t *def, const uint8_t *buf, size_t sz)
  {
    assert(sz <= N_REC_DATA);

    valid = true;
    proc_def = *def;

    if (buf != nullptr)
      _memcpy(data, buf, sz);
  }

  void reset() { valid = false; }
};

class recover_table_t
{
public:
  uint32_t magic = REC_MAGIC;

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
    tbl[pid].set(def, data, data_sz);
  }

  void reset_entry(pid_t pid) { tbl[pid].reset(); }
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
recover_table_t boot_table __recover_section__;

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
  flash::mmap(file, boot_table);
  if (boot_lev == boot_lev_t::FLASH) {
    boot_table = recover_table_t{};
    flash::store(file);
  }

  if (boot_lev == boot_lev_t::FLASH)
    sched::setup_procs();
  else if (boot_lev == boot_lev_t::BOOT)
    /* boot_table.recover(); */
    sched::setup_procs();
  else
    /* reset_table.recover(); */
    sched::setup_procs();
}

void set_rec(rec_lev_t rec_lev, const uint8_t *data, size_t data_sz)
{
  auto pid = curr_proc::pid();

  if (rec_lev == rec_lev_t::NONE) {
    reset_table.reset_entry(pid);

    boot_table.reset_entry(pid);
    flash::store(file);
  } else {
    auto proc_def = curr_proc::def();

    reset_table.set_entry(pid, proc_def, data, data_sz);

    if (rec_lev == rec_lev_t::BOOT) {
      boot_table.set_entry(pid, proc_def, data, data_sz);
      flash::store(file);
    }
  }
}

/** To be able to remove a process altogether feels too scary
 *  Maybe there should be a way to indicate a process to always run
 *  and also, maybe count "pin reset" as an actual restart, so it should call
 *  setup_procs
 * */

} // namespace recover
