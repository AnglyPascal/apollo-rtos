#include "header.h"

BEGIN_SUITE(waitlist)

volatile time_t start = 0, end = 0;
volatile bool end_proc = false;
constexpr size_t file_len = 16;

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

SYS_TEST(waitlist_basic)
{
  start = 0;
  end = 0;
  end_proc = false;

  uint32_t interval = 1;
  const uint32_t rounded = roundup(interval, waitlist::update_interval);
  bool result = true;

  while (interval <= rounded) {
    auto p0 = REG_PROC(proc0, interval);
    sched::wait(p0);
    result &= check_waitlist_interval(start, end, interval);
    interval++;
  }

  return result;
}

PROC(proc1, HIGH1, 128, param)
{
  start = timer::now();

  auto set_end = [](void *) {
    end = timer::now();
    end_proc = true;
  };
  waitlist::reg("set_end", *(uint32_t *)param, set_end);

  auto file = fram::open(test_fn, file_len, O_CREATE | O_WRITE);
  auto arr = (uint32_t *)file.mmap(file_len);
  for (int i = 0; i < 4; i++) {
    *arr++ = i;
  }
  file.store();
  file.close();

  while (!end_proc)
    ;
  end_proc = false;
}

SYS_TEST(waitlist_fs_op)
{
  start = 0;
  end = 0;
  end_proc = false;

  uint32_t interval = 1;
  bool result = true;

  auto p0 = REG_PROC(proc1, interval);
  sched::wait(p0);
  result &= check_waitlist_interval(start, end, interval);
  interval++;

  auto file = fram::open(test_fn, O_READ);
  auto arr = (uint32_t *)file.mmap(file_len);
  for (uint32_t i = 0; i < 4; i++)
    result &= (*arr++ == i);
  file.close();

  fram::remove(test_fn);

  return result;
}

END_SUITE()
