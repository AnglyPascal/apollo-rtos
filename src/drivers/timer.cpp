#include "drivers/timer.h"

#include "core/hardware.h"
#include "core/irq.h"
#include "core/sched.h"
#include "utility/debug.h"
#include "utility/profile.h"

namespace sched
{
void assert_stack();
void tick();
bool needs_swap();
} // namespace sched

namespace waitlist
{
bool increment(time_t millis);
void run();
} // namespace waitlist

namespace timer
{

constexpr time_t TICK = 1;

__always_inline__ inline void timer_init(volatile timer_t &TIMER)
{
  /* We use Timer 1 because its 16-bit mode is adequate for a clock with up to
   * 1us resolution and 1ms period, leaving the 32-bit Timer 0 for other
   * purposes. */
  TIMER.STOP = 1;
  TIMER.MODE = TIMER_MODE_Timer;
  TIMER.BITMODE = TIMER_BITMODE_16Bit;
  TIMER.PRESCALER = 4; /* 1MHz = 16MHz / 2^4 */
  TIMER.CLEAR = 1;
  TIMER.CC[0] = 1000 * TICK;
  TIMER.SHORTS = BIT(TIMER_COMPARE0_CLEAR);
  TIMER.INTENSET = BIT(TIMER_INT_COMPARE0);
  TIMER.START = 1;
}

void init()
{
  timer_init(TIMER1);
  enable_irq(TIMER1_IRQ);
  irq_priority(TIMER1_IRQ, IRQ_PRIO_HI);

  timer_init(TIMER2);
  enable_irq(TIMER2_IRQ);
  irq_priority(TIMER2_IRQ, IRQ_PRIO_HI);
}

volatile time_t MILLIS = 0;

namespace
{
constexpr size_t timer_stk_sz = 512;
uint8_t stk[timer_stk_sz];
uint8_t *stk_end = stk + timer_stk_sz;
} // namespace

__extern_C__ void pendsv_handler(void);

__always_inline__ inline void sched_invoke()
{
  if (MILLIS - sched::last_checked > sched::invoke_interval) {
    debug<FATAL>(DEFAULT "scheduler invoked. proc: " BOLD RED "%s\r\n" DEFAULT,
                 curr_proc::name());
    sched::last_checked = MILLIS;
    if (sched::needs_swap())
      pendsv_handler();
  }
}

__extern_C__ void timer1_handler(void)
{
  if (TIMER1.COMPARE[0]) {
    MILLIS += TICK;
    TIMER1.COMPARE[0] = 0;
  }

  sched::assert_stack();

  if (waitlist::increment(MILLIS)) {
    intr_guard guard;

    auto prev_stk = (uint8_t *)get_msp();
    set_msp(stk_end); // setup temporary stk to run waitlist tasks
    waitlist::run();
    set_msp(prev_stk);
  }

  sched_invoke();
}

volatile time_t debug_ticks = 0;

__extern_C__ void timer2_handler(void)
{
  if (TIMER2.COMPARE[0]) {
    debug_ticks += TICK;
    TIMER2.COMPARE[0] = 0;
  }

  sched::tick();
  profile::tick();
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
