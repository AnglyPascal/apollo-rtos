#include "hardware.h"
#include "irq.h"
#include "lib.h"
#include "memory.h"
#include "sched.h"
#include "serial.h"
#include "waitlist.h"

namespace timer
{

constexpr time_t TICK = 1;

void init()
{
  /* We use Timer 1 because its 16-bit mode is adequate for a clock with up to
   * 1us resolution and 1ms period, leaving the 32-bit Timer 0 for other
   * purposes. */
  TIMER1.STOP = 1;
  TIMER1.MODE = TIMER_MODE_Timer;
  TIMER1.BITMODE = TIMER_BITMODE_16Bit;
  TIMER1.PRESCALER = 4; /* 1MHz = 16MHz / 2^4 */
  TIMER1.CLEAR = 1;
  TIMER1.CC[0] = 1000 * TICK;
  TIMER1.SHORTS = BIT(TIMER_COMPARE0_CLEAR);
  TIMER1.INTENSET = BIT(TIMER_INT_COMPARE0);
  TIMER1.START = 1;

  enable_irq(TIMER1_IRQ);
}

time_t MILLIS = 0;

namespace
{
constexpr size_t timer_stack_sz = 512;
uint8_t stack[timer_stack_sz];
void *stack_end = stack + timer_stack_sz;
} // namespace

/* extern "C" void *timer_body(void *old_stk) */
extern "C" void *timer1_handler(void *old_stk)
{
  void *curr;

  asm volatile("mrs %[sp], msp" : [sp] "=r"(curr));
  asm volatile("msr msp, %[stk]" : : [stk] "r"(stack_end));

  if (TIMER1.COMPARE[0]) {
    MILLIS += TICK;
    TIMER1.COMPARE[0] = 0;
  }

  disable_irq(TIMER1_IRQ);
  if ((MILLIS & (waitlist::update_interval - 1)) == 0) {
    waitlist::run();
  }
  enable_irq(TIMER1_IRQ);

  asm volatile("msr msp, %[stk]" : : [stk] "r"(curr));

  // FIXME: turned off schedule invoker
  /* return sched::invoke(old_stack, millis); */
  return old_stk;
}

} // namespace timer
