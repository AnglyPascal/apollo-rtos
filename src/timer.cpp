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

__always_inline__
inline void timer1_init()
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

void init()
{
  timer1_init();
}

volatile time_t MILLIS = 0;

namespace
{
constexpr size_t timer_stk_sz = 512;
uint8_t stk[timer_stk_sz];
uint8_t *stk_end = stk + timer_stk_sz;
} // namespace

__extern_C__
void pendsv_handler(void);

__extern_C__
void timer1_handler(void)
{
  if (TIMER1.COMPARE[0]) {
    MILLIS += TICK;
    TIMER1.COMPARE[0] = 0;
  }

  if ((MILLIS & (waitlist::update_interval - 1)) == 0) {
    disable_irq(TIMER1_IRQ);

    auto prev_stk = (uint8_t *)get_msp();
    set_msp(stk_end); // setup temporary stk to run waitlist tasks
    waitlist::run();
    set_msp(prev_stk);

    enable_irq(TIMER1_IRQ);
  }

  if (MILLIS - sched::_last_checked() > 2043) {
    debug<FATAL>("scheduler invoked\r\n");
    pendsv_handler();
  }
}

/** Scheduler invoker inside timer1_handler:
 *
 * when timer1_handler calls pensdsv_handler, the current process stack
 * contains
 *   - the hardare saved registers
 *   - whatever timer1_handlers pushed on the stack
 * and the lr register points to the exit from timer1_handler code
 *
 * So when pendsv_handler calls isave, it stores the manually stored registers
 * on top of the stack contents.
 *
 * Now when this process is resumed, the exit code of timer1_handler pops
 * whatever it pushed to the stack before exiting from the timer1_handler inside
 * that original process. That's why the original process sees its desired
 * registers when it finally resumes.
 */

} // namespace timer
