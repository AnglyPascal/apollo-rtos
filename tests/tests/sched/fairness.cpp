#include "header.h"

namespace TEST_SUITE(sched_basic)
{
uint32_t results[3];
size_t idx = 0;

PROC(proc0, LOW2, 8, param) { results[idx++] = *(uint32_t *)param; }
PROC(proc1, MID2, 8, param) { results[idx++] = *(uint32_t *)param; }
PROC(proc2, HIGH2, 8, param) { results[idx++] = *(uint32_t *)param; }

SYS_TEST(sched_basic_priority)
{
  auto p0 = REG_PROC(proc0, 2);
  auto p1 = REG_PROC(proc1, 1);
  auto p2 = REG_PROC(proc2, 0);

  sched::wait(p0, p1, p2);

  return results[0] == 0 && results[1] == 1 && results[2] == 2;
}
} // namespace TEST_SUITE(sched_basic)

namespace TEST_SUITE(sched_sleep)
{
volatile time_t start = 0, end = 0;
constexpr size_t interval = 4;

uint32_t results[3];
size_t idx = 0;

PROC(proc0, HIGH2, 8, param)
{
  auto [a, b] = *(pair<uint32_t, uint32_t> *)param;
  results[idx++] = a;

  start = timer::now();
  sched::sleep(interval);
  end = timer::now();

  results[idx++] = b;
}

PROC(proc1, MID2, 8, param) { results[idx++] = *(uint32_t *)param; }

SYS_TEST(sched_sleep)
{
  auto p0 = REG_PROC(proc0, pair<uint32_t, uint32_t>{0, 2});
  auto p1 = REG_PROC(proc1, 1);

  sched::wait(p0, p1);

  return results[0] == 0 && results[1] == 1 && results[2] == 2 &&
         (end - start) <= roundup(interval, waitlist::update_interval);
}
} // namespace TEST_SUITE(sched_sleep)

namespace TEST_SUITE(sched_fairness)
{
volatile uint32_t result[6];
size_t idx = 0;

PROC(proc0, MID1, 8, param)
{
  auto p = *(uint32_t *)param;

  result[idx++] = p;
  delay_loop(10000);
  sched::sleep(1);
  result[idx++] = p;
}

SYS_TEST(sched_fairness)
{
  auto p1 = REG_PROC(proc0, 1);
  auto p2 = REG_PROC(proc0, 2);
  auto p3 = REG_PROC(proc0, 3);

  sched::wait(p1, p2, p3);

  bool res = true;
  res &= result[0] == 1 && result[3] == 1;
  res &= result[1] == 2 && result[4] == 2;
  res &= result[2] == 3 && result[5] == 3;

  return res;
}
} // namespace TEST_SUITE(sched_fairness)
