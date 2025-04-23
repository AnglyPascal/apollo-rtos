#include "header.h"

BEGIN_SUITE(mutex)

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

SYS_TEST(test_mutex_exclusion)
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

SYS_TEST(test_priority_inheritance)
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
