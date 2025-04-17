#include "header.h"

namespace
{
bool check_waitlist_itnerval(time_t start, time_t end, time_t interval)
{
  auto rounded_future = roundup(start + interval, waitlist::update_interval);
  interval = roundup(rounded_future - start, waitlist::update_interval);
  auto rt_interval = roundup(end - start, waitlist::update_interval);
  return interval == rt_interval;
}
} // namespace

namespace TEST_SUITE(sched_waitlist)
{
volatile time_t start = 0, end = 0;
volatile bool end_proc = false;

PROC(proc0, HIGH1, 8, param)
{
  start = timer::now();

  auto set_end = [](void *) {
    end = timer::now();
    end_proc = true;
  };
  waitlist::reg("set_end", *(uint32_t *)param, set_end);

  while (!end_proc)
    ;
  end_proc = false;
}

SYS_TEST(sched_waitlist)
{
  uint32_t interval = 1;
  const uint32_t rounded = roundup(interval, waitlist::update_interval);
  bool result = true;

  while (interval <= rounded) {
    auto p0 = REG_PROC(proc0, interval);
    sched::wait(p0);
    result &= check_waitlist_itnerval(start, end, interval);
    interval++;
  }

  return result;
}
} // namespace TEST_SUITE(sched_waitlist)

namespace TEST_SUITE(sched_waitlist_fs_op)
{
volatile time_t start = 0, end = 0;
volatile bool end_proc = false;

PROC(proc0, HIGH1, 8, param)
{
  start = timer::now();

  auto set_end = [](void *) {
    end = timer::now();
    end_proc = true;
  };
  waitlist::reg("set_end", *(uint32_t *)param, set_end);

  auto file = fram::open(test_fn, 16, O_CREATE | O_WRITE);
  auto arr = (uint32_t *)file.mmap(16);
  for (int i = 0; i < 4; i++) {
    *arr++ = i;
  }
  file.store();
  file.close();

  while (!end_proc)
    ;
  end_proc = false;
}

SYS_TEST(sched_waitlist_fs_op)
{
  uint32_t interval = 1;
  // const uint32_t rounded = roundup(interval, waitlist::update_interval);
  bool result = true;

  auto p0 = REG_PROC(proc0, interval);
  sched::wait(p0);
  result &= check_waitlist_itnerval(start, end, interval);
  interval++;

  auto file = fram::open(test_fn, 16, O_READ);
  auto arr = (uint32_t *)file.mmap(16);
  for (uint32_t i = 0; i < 4; i++)
    result &= (*arr++ == i);
  file.store();
  file.close();

  fram::remove(test_fn);

  return result;
}
} // namespace TEST_SUITE(sched_waitlist_fs_op)
