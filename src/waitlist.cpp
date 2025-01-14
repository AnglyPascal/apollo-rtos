#include "waitlist.h"
#include "debug.h"
#include "hardware.h"
#include "irq.h"
#include "lib.h"
#include "memory.h"
#include "serial.h"

#include <utility>

namespace waitlist
{

namespace
{

struct waitlist_t {
  string name;
  time_t remaining;
  void (*func)(void *);
  void *param;
};

constexpr uint8_t N_WAITLIST = 16;
waitlist_t waitlist[N_WAITLIST];
volatile uint8_t list_sz = 0;

inline void swap(waitlist_t &lhs, waitlist_t &rhs)
{
  std::swap(lhs.name, rhs.name);
  std::swap(lhs.remaining, rhs.remaining);
  std::swap(lhs.func, rhs.func);
  std::swap(lhs.param, rhs.param);
}

} // namespace

void reg(string name, time_t interval, void (*func)(void *), void *param)
{
  if (list_sz == N_WAITLIST) {
    debug<FATAL>("!! NO SPACE IN WAITLIST\r\n");
    return;
  }

  interval = roundup(interval, update_interval);

  uint8_t i = 0;
  time_t prev = 0;
  while (i < list_sz && prev + waitlist[i].remaining <= interval) {
    prev += waitlist[i++].remaining;
  }

  waitlist[i].remaining += prev - interval;

  auto j = i + 1;
  while (j <= list_sz) {
    swap(waitlist[i], waitlist[j++]);
  }

  waitlist[i] = {name, interval - prev, func, param};
  list_sz++;
}

void run()
{
  intr_guard guard;

  if (list_sz == 0)
    return;

  // no carry, because of roundup
  waitlist[0].remaining -= update_interval;

  // when two tasks are to be done at the same time,
  // the relative order is first come first serve
  while (list_sz > 0 && waitlist[0].remaining == 0) {
    auto func = waitlist[0].func;
    auto param = waitlist[0].param;

    // TODO: improve efficiency
    for (int i = 1; i < list_sz; i++) {
      swap(waitlist[i], waitlist[i - 1]);
    }

    list_sz--;
    func(param);
  }
}

void trace()
{
  printf("waitlist:\r\n");
  for (size_t i = 0; i < list_sz; i++) {
    auto &task = waitlist[i];
    printf("\t%s\r\n\t\tinterval: %u\r\n", task.name.str, task.remaining);
  }
}

} // namespace waitlist
