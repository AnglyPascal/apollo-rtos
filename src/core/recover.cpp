#include "core/recover.h"

#include "core/fs.h"
#include "core/memory.h"
#include "core/sched.h"
#include "utility/debug.h"

#define REC_MAGIC 0xbabedadd

class recover_table_t
{
private:
  enum state_t : uint8_t {
    EMPTY,
    ACQUIRED,
    IN_USE,
  };

  struct entry_t {
    state_t state = EMPTY;
    proc_def_t proc_def;
    uint8_t data[N_REC_DATA] = {0xFF};
  };

  // FIXME: incr N_PROCS to demonstrate that multi-blk files are not working
  entry_t tbl[N_PROCS] = {};
  static_assert(sizeof(tbl) < 1024);

public:
  rec_id_t get_rec_id()
  {
    for (uint8_t i = 0; i < N_PROCS; i++) {
      if (tbl[i].state == EMPTY) {
        tbl[i].state = ACQUIRED;
        return i;
      }
    }
    assert(false, "no recover table entry free\r\n");
    return null_rec_id;
  }

  void recover()
  {
    for (auto &entry : tbl) {
      if (entry.state == IN_USE) {
        entry.state = EMPTY;
        sched::reg_proc(&entry.proc_def, entry.data);
      } else if (entry.state == ACQUIRED) {
        entry.state = EMPTY;
      }
    }
  }

  void set_entry(uint8_t entry_id, const proc_def_t *def, const uint8_t *data,
                 size_t data_sz)
  {
    auto &entry = tbl[entry_id];
    assert(entry.state != EMPTY, "%d, %s, %d\r\n", entry_id, def->name.str,
           entry.state);

    entry.state = IN_USE;
    entry.proc_def = *def;

    if (data != nullptr) {
      assert(data_sz <= N_REC_DATA);
      _memcpy(entry.data, data, data_sz);
    }
  }

  void reset_entry(uint8_t entry_id)
  {
    auto &entry = tbl[entry_id];
    entry.state = EMPTY;
  }
};

namespace sched
{
void setup_procs(void);
}

namespace curr_proc
{
proc_def_t *def();
uint8_t rec_id();
void set_rec_id(rec_id_t id);
} // namespace curr_proc

namespace recover
{
namespace
{
recover_table_t reset_table __recover_section__;
constexpr fn_t recover_fn = 0;
flash::file_t file;
} // namespace

void init() 
{
  auto boot_lev = boot::lev();

  // FIXME: wtf is this stack usage bruh
  file = flash::open(recover_fn, sizeof(recover_table_t), O_WRITE | O_CREATE);
  flash::mmap(file, reset_table);

  // TODO: hardware restart if the recover table is corrupt
  if (boot_lev != boot_lev_t::RESET) {
    reset_table = recover_table_t{};
    sched::setup_procs();
  } else {
    reset_table.recover();
  }

  flash::store(file);
}

void set_rec(rec_lev_t rec_lev, const uint8_t *data, size_t data_sz)
{

  if (rec_lev == rec_lev_t::NONE) {
    auto entry_id = curr_proc::rec_id();
    if (entry_id != null_rec_id)
      reset_table.reset_entry(entry_id);
  } else {
    auto entry_id = reset_table.get_rec_id();
    curr_proc::set_rec_id(entry_id);
    reset_table.set_entry(entry_id, curr_proc::def(), data, data_sz);
  }

  flash::store(file);
}

/** To be able to remove a process altogether feels too scary
 *  Maybe there should be a way to indicate a process to always run
 *  and also, maybe count "pin reset" as an actual restart, so it should call
 *  setup_procs
 * */

} // namespace recover
