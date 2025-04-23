#include "header.h"

BEGIN_SUITE(sched)

// Test priority-based execution order
PROC(high_pri_proc, HIGH1, 0, param)
{
  auto counter = *static_cast<int **>(param);
  *counter = *counter * 10 + 1;
}

PROC(low_pri_proc, MID2, 0, param)
{
  auto counter = *static_cast<int **>(param);
  *counter = *counter * 10 + 2;
}

SYS_TEST(test_priority_order)
{
  int counter = 0;
  volatile auto p_high = REG_PROC(high_pri_proc, &counter);
  volatile auto p_low = REG_PROC(low_pri_proc, &counter);
  sched::wait(p_high, p_low);

  return counter == 12;
}

// Test same-priority round-robin
PROC(proc_a, MID1, 0, param)
{
  auto counter = *static_cast<int **>(param);
  delay_loop(500);
  *counter = *counter * 10 + 1;
  sched::sleep(5);
}

PROC(proc_b, MID1, 0, param)
{
  auto counter = *static_cast<int **>(param);
  delay_loop(500);
  *counter = *counter * 10 + 2;
  sched::sleep(5);
}

PROC(proc_c, MID1, 0, param)
{
  auto counter = *static_cast<int **>(param);
  delay_loop(500);
  *counter = *counter * 10 + 3;
  sched::sleep(5);
}

SYS_TEST(test_round_robin)
{
  int counter = 0;
  auto p1 = REG_PROC(proc_a, &counter);
  auto p2 = REG_PROC(proc_b, &counter);
  auto p3 = REG_PROC(proc_c, &counter);
  sched::wait(p1, p2, p3);

  return counter == 123; // Expected execution order: 1 then 2
}

// Test channel-based wakeup
PROC(waiting_proc, HIGH1, 5, param)
{
  auto chan = *static_cast<chan_t<1> **>(param);
  sched::wait(*chan);
}

PROC(notifying_proc, MID2, 5, param)
{
  auto chan = *static_cast<chan_t<1> **>(param);
  sched::notify(*chan);
}

SYS_TEST(test_channel_wakeup)
{
  chan_t<1> wake_chan;

  auto p_wait = REG_PROC(waiting_proc, &wake_chan);
  auto p_notify = REG_PROC(notifying_proc, &wake_chan);
  sched::wait(p_wait, p_notify);

  return wake_chan.empty(); // Channel should be empty after successful wake
}

// Test timer-based wakeup
PROC(sleeping_proc, MID2, 5, param)
{
  auto start = timer::now();
  sched::sleep(10);
  **static_cast<size_t **>(param) = timer::now() - start;
}

SYS_TEST(test_timer_duration)
{
  size_t duration = 0;
  auto p = REG_PROC(sleeping_proc, &duration);
  sched::wait(p);
  return duration >= 10; // Should sleep at least 10ms
}

END_SUITE()
