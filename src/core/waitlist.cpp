#include "core/waitlist.h"
#include "core/irq.h"
#include "utility/allocator.h"
#include "utility/debug.h"

namespace waitlist
{

namespace
{
struct waitlist_t {
  string name;
  time_t remaining = 0;
  runnable_t func = nullptr;
  void *param = nullptr;

  waitlist_t *next = nullptr;
};

constexpr uint8_t N_WAITLIST = 16;
waitlist_t store[N_WAITLIST];
size_t i = 0;

waitlist_t freelist_hd{};
waitlist_t waitlist_hd{};

inline waitlist_t *alloc()
{
  if (i == N_WAITLIST) {
    return nullptr;
  }

  if (freelist_hd.next != nullptr) {
    auto ptr = freelist_hd.next;
    freelist_hd.next = ptr->next;
    return ptr;
  }

  return store + i++;
}

inline void dealloc(waitlist_t *ptr)
{
  ptr->next = freelist_hd.next;
  freelist_hd.next = ptr;
}
} // namespace

void reg(string name, time_t interval, runnable_t func, void *param)
{
  auto task = alloc();
  assert(task != nullptr);

  interval = roundup(interval, update_interval);

  auto head = &waitlist_hd;
  time_t prev = 0;
  while (head->next != nullptr && prev + head->next->remaining <= interval) {
    prev += head->next->remaining;
    head = head->next;
  }

  auto remaining = interval - prev;

  if (head->next != nullptr) {
    head->next->remaining -= remaining;
  }

  *task = {name, remaining, func, param, head->next};
  head->next = task;
}

void run()
{
  intr_guard guard;

  auto head = &waitlist_hd;
  if (head->next != nullptr)
    head->next->remaining -= update_interval;

  while (head->next != nullptr && head->next->remaining == 0) {
    auto task = head->next;
    head->next = task->next;

    auto [name, remaining, func, param, next] = *task;
    dealloc(task);
    func(param);
  }
}

void trace()
{
  if (waitlist_hd.next == nullptr)
    debug<INFO>("  waitlist: NONE\r\n");
  else
    debug<INFO>("  waitlist:\r\n");

  for (auto head = &waitlist_hd; head->next != nullptr; head = head->next) {
    debug<INFO>("  |  %s\r\n  |    interval: %u\r\n", head->next->name.str,
                head->next->remaining);
  }
}

} // namespace waitlist
