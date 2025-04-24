#pragma once

#include "core/sched.h"
#include "core/test.h"
#include "core/waitlist.h"
#include "drivers/display.h"
#include "drivers/timer.h"
#include "fs/fs.h"
#include "utility/debug.h"

inline constexpr fn_t test_fn = 3;

inline bool check_waitlist_itnerval(time_t start, time_t end, time_t interval)
{
  auto rounded_future = roundup(start + interval, waitlist::update_interval);
  interval = roundup(rounded_future - start, waitlist::update_interval);
  auto rt_interval = roundup(end - start, waitlist::update_interval);
  return interval == rt_interval;
}
