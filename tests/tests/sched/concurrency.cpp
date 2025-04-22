#include "header.h"

BEGIN_SUITE(sched_concurrency)

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

SYS_TEST(sleep_notify)
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

END_SUITE()
