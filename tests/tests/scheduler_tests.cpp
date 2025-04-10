#include "core/sched.h"
#include "core/test.h"
#include "drivers/display.h"
#include "utility/debug.h"

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
uint32_t results[3];
size_t idx = 0;

PROC(proc0, HIGH2, 8, param)
{
  auto [a, b] = *(pair<uint32_t, uint32_t> *)param;
  results[idx++] = a;
  sched::sleep(10);
  results[idx++] = b;
}
PROC(proc1, MID2, 8, param) { results[idx++] = *(uint32_t *)param; }

SYS_TEST(sched_sleep)
{
  auto p0 = REG_PROC(proc0, pair<uint32_t, uint32_t>{0, 2});
  auto p1 = REG_PROC(proc1, 1);

  sched::wait(p0, p1);

  return results[0] == 0 && results[1] == 1 && results[2] == 2;
}
} // namespace TEST_SUITE(sched_sleep)

namespace TEST_SUITE(sched_sleep_notify)
{
uint32_t results[6];
size_t idx = 0;

uint32_t shared_var = 0;
chan_t<1> server, client;

constexpr uint32_t server_val = 1;
constexpr uint32_t client_val = 2;

PROC(server, HIGH2, 8, param)
{
  int n = 3;
  while (n-- > 0) {
    results[idx++] = server_val;
    shared_var = *(uint32_t *)param;

    sched::notify(client);
    sched::wait(server);
  }
}

PROC(client, MID2, 8, param)
{
  int n = 3;
  while (n-- > 0) {
    results[idx++] = shared_var;
    shared_var = 0;

    sched::notify(server);
    if (n > 0)
      sched::wait(client);
  }
}

SYS_TEST(sched_sleep_notify)
{
  auto p0 = REG_PROC(server, client_val);
  auto p1 = REG_PROC(client, nullptr);

  sched::wait(p0, p1);

  bool res = true;
  int i = 0;
  while (i < 6)
    res &= (results[i++] == server_val && results[i++] == client_val);
  return res;
}
} // namespace TEST_SUITE(sched_sleep_notify)
