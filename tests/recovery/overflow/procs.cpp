#include "core/memory.h"
#include "core/recover.h"
#include "core/sched.h"
#include "utility/args.h"

namespace
{
constexpr size_t n_mid = 4, n_high = 4, n_low = 4;
constexpr uint32_t v_low = 10, v_mid = 100, v_high = 1000;

uint32_t exp[] = {
    // high priority procs run first
    v_high, v_high + 1, v_high + 2, v_high + 3, //
    // mid priority runs, third one causes soft reset
    v_mid, v_mid + 1, v_mid + 2, //
                                 //
    // after reset, the 4 high prio and 3 mid prio are restarted
    v_high, v_high + 1, v_high + 2, v_high + 3, //
    v_mid, v_mid + 1, v_mid + 2,                //
    v_low, v_low + 1, v_low + 2,                //
};
constexpr size_t N = sizeof(exp) / sizeof(exp[0]);
uint32_t res[N] __recover_section__ = {0};

size_t idx __recover_section__ = 0;
size_t n_reset __recover_section__ = 0;

PROC(low_proc, LOW4, 128, p)
{
  auto v = *(uint32_t *)p;
  res[idx++] = v;
  while (1)
    sched::yield();
}

PROC(high_proc, HIGH4, 128, p)
{
  auto v = *(uint32_t *)p;
  recover::guard_proc guard{POWER_OFF, v};

  res[idx++] = v;
  while (1)
    sched::sleep(1000);
}

PROC(mid_proc, MID4, 128, p)
{
  auto v = *(uint32_t *)p;
  recover::guard_proc guard{POWER_OFF, v};

  res[idx++] = v;

  REG_PROC(low_proc, v - v_mid + v_low);
  while (1)
    sched::sleep(1000);
}

PROC(test, HIGHEST, 128, p)
{
  n_reset++;
  recover::guard_proc guard{POWER_OFF, nullptr};

  while (1) {
    sched::sleep(200);

    if (n_reset == 1)
      continue;

    bool result = idx == (sizeof(exp) / sizeof(exp[0]));
    for (size_t i = 0; i < idx; i++)
      result &= res[i] == exp[i];

    kprintf("test %s\r\n",
            result ? GREEN "passed" DEFAULT : RED "failed" DEFAULT);
    break;
  }
}

} // namespace

namespace sched
{
void setup_services(void) {}

void setup_startups(void)
{
  REG_PROC(test, nullptr);

  for (uint32_t i = 0; i < n_low; i++)
    REG_PROC(low_proc, v_low + i);

  for (uint32_t i = 0; i < n_mid; i++)
    REG_PROC(mid_proc, v_mid + i);

  for (uint32_t i = 0; i < n_high; i++)
    REG_PROC(high_proc, v_high + i);
}
} // namespace sched
