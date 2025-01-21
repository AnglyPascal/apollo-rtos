#include "waitlist.h"
#include "allocator.h"
#include "debug.h"
#include "irq.h"
#include "memory.h"
#include "serial.h"

#include <utility>

namespace waitlist
{

namespace
{
struct waitlist_t {
  string name;
  time_t remaining = 0;
  void (*func)(void *) = nullptr;
  void *param = nullptr;

  waitlist_t *next = nullptr;
};

constexpr uint8_t N_WAITLIST = 16;
waitlist_t store[N_WAITLIST];
int i = 0;
waitlist_t freelist_head{};

inline waitlist_t *alloc()
{
  if (i == N_WAITLIST) {
    return nullptr;
  }

  if (freelist_head.next != nullptr) {
    auto ptr = freelist_head.next;
    freelist_head.next = ptr->next;
    return ptr;
  }

  return store + i++;
}

inline void dealloc(waitlist_t *ptr)
{
  ptr->next = freelist_head.next;
  freelist_head.next = ptr;
}

waitlist_t list_head{};
} // namespace

void reg(string name, time_t interval, void (*func)(void *), void *param)
{
  auto task = alloc();
  assert(task != nullptr);

  interval = roundup(interval, update_interval);

  auto head = &list_head;
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

  auto head = &list_head;
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
  printf("waitlist:\r\n");
  for (auto head = &list_head; head->next != nullptr; head = head->next) {
    printf("\t%s\r\n\t\tinterval: %u\r\n", head->next->name.str,
           head->next->remaining);
  }
}

} // namespace waitlist
