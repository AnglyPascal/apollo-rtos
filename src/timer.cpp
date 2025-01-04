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

volatile time_t MILLIS = 0;

namespace
{
constexpr size_t timer_stk_sz = 512;
uint8_t stk[timer_stk_sz];
uint8_t *stk_end = stk + timer_stk_sz;
uint8_t *prev_stk;
} // namespace

__extern_C__
__attribute__((optimize("O1"))) // NOTE: works for now
void *timer_body(void *old_stk)
{
  asm volatile("mrs %[stk], msp" : [stk] "=r"(prev_stk));

  if (TIMER1.COMPARE[0]) {
    MILLIS += TICK;
    TIMER1.COMPARE[0] = 0;
  }

  if ((MILLIS & (waitlist::update_interval - 1)) == 0) {
    disable_irq(TIMER1_IRQ);

    // setup temporary stk to run waitlist tasks
    asm volatile("msr msp, %[stk]" : : [stk] "r"(stk_end));
    waitlist::run();

    enable_irq(TIMER1_IRQ);
  }

  asm volatile("msr msp, %[stk]" : : [stk] "r"(prev_stk));

  // FIXME: turned off schedule invoker
  /* return sched::invoke(old_stk, millis); */
  return old_stk;
}

} // namespace timer
