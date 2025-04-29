#include "header.h"


BEGIN_SUITE(sched_fairness)

volatile uint32_t result[6];
size_t idx = 0;
constexpr uint32_t v1 = 0xbabebebe, v2 = 0xdeadbeef, v3 = 0xbecedece;

PROC(proc0, MID1, 8, param)
{
  auto v = *(uint32_t *)param;
  result[idx++] = v;
  delay_loop(10000);
  sched::sleep(1);
  result[idx++] = v;
}

SYS_TEST(sleep_fairness)
{
  auto p1 = REG_PROC(proc0, v1);
  auto p2 = REG_PROC(proc0, v2);
  auto p3 = REG_PROC(proc0, v3);

  sched::wait(p1, p2, p3);

  bool res = true;
  res &= result[0] == v1 && result[3] == v1;
  res &= result[1] == v2 && result[4] == v2;
  res &= result[2] == v3 && result[5] == v3;

  return res;
}

PROC(proc1, MID3, 8, param)
{
  auto v = *(uint32_t *)param;
  while (idx < 6) {
    result[idx++] = v;
    sched::yield();
  }
}

SYS_TEST(yield_fairness)
{
  idx = 0;

  auto p1 = REG_PROC(proc1, v1);
  auto p2 = REG_PROC(proc1, v2);
  auto p3 = REG_PROC(proc1, v3);

  sched::wait(p1, p2, p3);

  bool res = true;
  res &= result[0] == v1 && result[3] == v1;
  res &= result[1] == v2 && result[4] == v2;
  res &= result[2] == v3 && result[5] == v3;

  return res;
}

END_SUITE()

