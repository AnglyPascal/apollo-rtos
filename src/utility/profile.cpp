#include "utility/profile.h"

#include "core/types.h"
#include "drivers/timer.h"
#include "utility/debug.h"

namespace profile
{

static constexpr size_t null_id = -1;
volatile size_t curr_entry = null_id;
entry_t tbl[N_PROFILES] = {};

profile_guard::profile_guard(size_t _entry_id, const char *name)
    : entry_id{curr_entry}
{
  tbl[_entry_id].func_name = name;
  curr_entry = _entry_id;
}

profile_guard::~profile_guard() { curr_entry = entry_id; }

void trace()
{
  debug<INFO>("profiles:\r\n");
  for (auto &entry : tbl) {
    if (entry.func_name != nullptr) {
      debug<INFO>("  |  %s: %f%, %d\r\n", entry.func_name,
                  rational_t{(int32_t)entry.ticks * 100, timer::debug_ticks},
                  entry.ticks);
    }
  }
}

void tick()
{
  if (curr_entry != null_id) {
    tbl[curr_entry].ticks++;
  }
}
} // namespace profile
