#include "core/waitlist.h"

#include "core/irq.h"
#include "core/types.h"
#include "drivers/timer.h"
#include "utility/debug.h"

namespace waitlist
{

namespace
{
struct waitlist_t {
  const char *name;
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

void reg(const char *name, time_t interval, runnable_t func, void *param)
{
  intr_guard guard{TIMER1_IRQ};

  auto task = alloc();
  assert(task != nullptr, S_RESET);

  auto now = timer::now();
  auto rounded_future = roundup(now + interval, update_interval);
  interval = roundup(rounded_future - now, update_interval);

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

time_t last = 0;

// return if a waitlist task is ready to run
bool increment(time_t millis)
{
  if (millis % update_interval != 0)
    return false;

  auto head = &waitlist_hd;
  if (head->next != nullptr)
    head->next->remaining -= update_interval;
  return head->next != nullptr && head->next->remaining == 0;
}

void run()
{
  auto head = &waitlist_hd;
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
  debug<INFO>(BOLD "waitlist: ");
  if (waitlist_hd.next == nullptr)
    return debug<INFO>(YELLOW "NONE" DEFAULT "\r\n");

  debug<INFO>(DEFAULT "\r\n");
  for (auto head = &waitlist_hd; head->next != nullptr; head = head->next) {
    debug<INFO>("  |  " BOLD CYAN "%s" DEFAULT //
                " : (" BLUE "%u" DEFAULT ")\r\n",
                head->next->name, head->next->remaining);
  }
}

} // namespace waitlist
