#include "waitlist.h"
#include "hardware.h"
#include "lib.h"
#include "memory.h"
#include "serial.h"

#include <utility>

namespace waitlist
{

namespace
{

struct waitlist_t {
  time_t remaining;
  void (*func)(void*);
  void *param;
};

} // namespace

constexpr uint8_t N_WAITLIST = 8;
waitlist_t list[N_WAITLIST];
uint8_t list_end = 0;

void reg(time_t interval, void (*func)(void*), void *param)
{
  interval = roundup(interval, update_interval);

  if (list_end == N_WAITLIST) {
    // panic
    printf("no available space for more waitlists\n");
  }

  uint8_t i = 0;
  time_t prev = 0;
  while (i < list_end && prev + list[i].remaining <= interval) {
    prev += list[i++].remaining;
  }

  list[i].remaining += prev - interval;

  auto j = i + 1;
  while (j <= list_end) {
    std::swap(list[i], list[j++]);
  }

  list[i] = {interval - prev, func, param};
  list_end++;
}

void run()
{
  if (list_end == 0)
    return;

  // no carry, because of roundup
  list[0].remaining -= update_interval;

  // when two tasks are to be done at the same time,
  // the relative order is first come first serve
  while (list_end > 0 && list[0].remaining == 0) {
    auto func = list[0].func;
    auto param = list[0].param;

    for (int i = 1; i < list_end; i++) {
      std::swap(list[i], list[i - 1]);
    }

    list_end--;

    func(param);
  }
}

} // namespace waitlist
