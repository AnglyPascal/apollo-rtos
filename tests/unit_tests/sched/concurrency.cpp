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

namespace
{
struct {
  pid_t order[4];
  size_t i = 0;
  void reset()
  {
    for (auto &o : order)
      o = -1;
    i = 0;
  }
  pid_t &operator[](size_t idx) { return order[idx]; }
} ord;
} // namespace

// Test mutual exclusion
PROC(mutex_proc1, MID2, 5, param)
{
  auto mtx = *static_cast<mutex<2> **>(param);

  mtx->lock();
  ord[ord.i++] = curr_proc::pid();

  sched::sleep(5);

  ord[ord.i++] = curr_proc::pid();
  mtx->unlock();
}

PROC(mutex_proc2, MID2, 5, param)
{
  auto mtx = *static_cast<mutex<2> **>(param);

  mtx->lock();
  ord[ord.i++] = curr_proc::pid();

  ord[ord.i++] = curr_proc::pid();
  mtx->unlock();
}

SYS_TEST(mutex_exclusion)
{
  ord.reset();

  mutex<2> mtx;
  auto p1 = REG_PROC(mutex_proc1, &mtx);
  auto p2 = REG_PROC(mutex_proc2, &mtx);
  sched::wait(p1, p2);

  return ord[0] == ord[1] && ord[2] == ord[3] && ord.i == 4;
}

// Test mutex with priority inheritance
PROC(high_pri_mutex_proc, HIGH1, 0, param)
{
  auto mtx = *static_cast<mutex<2> **>(param);

  mtx->lock();
  ord[ord.i++] = curr_proc::pid();

  sched::sleep(5);

  ord[ord.i++] = curr_proc::pid();
  mtx->unlock();
}

PROC(low_pri_mutex_proc, MID2, 0, param)
{
  auto mtx = *static_cast<mutex<2> **>(param);

  mtx->lock();
  ord[ord.i++] = curr_proc::pid();

  ord[ord.i++] = curr_proc::pid();
  mtx->unlock();
}

SYS_TEST(mutex_priority)
{
  ord.reset();

  mutex<2> mtx;
  auto p_high = REG_PROC(high_pri_mutex_proc, &mtx);
  auto p_low = REG_PROC(low_pri_mutex_proc, &mtx);
  sched::wait(p_high, p_low);

  return ord[0] == p_high && ord[0] == ord[1] && ord[2] == p_low &&
         ord[2] == ord[3] && ord.i == 4;
}

END_SUITE()

