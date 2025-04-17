#pragma once

#include "core/hardware.h"
#include "core/types.h"

/* Interrupts */
#define SVC_IRQ -5
#define PENDSV_IRQ -2
#define RADIO_IRQ 1
#define UART_IRQ 2
#define I2C0_IRQ 3
#define SPI0_IRQ 3
#define I2C1_IRQ 4
#define SPI1_IRQ 4
#define GPIOTE_IRQ 6
#define ADC_IRQ 7
#define TIMER0_IRQ 8
#define TIMER1_IRQ 9
#define TIMER2_IRQ 10
#define RTC0_IRQ 11
#define TEMP_IRQ 12
#define RNG_IRQ 13
#define RTC1_IRQ 17

#define N_INTERRUPTS 32

#define IRQ_PRIO_HI 0
#define IRQ_PRIO_M1 1
#define IRQ_PRIO_M2 2
#define IRQ_PRIO_LO 3

/* NVIC SETUP FUNCTIONS */

/* On Cortex-M0, only the top two bits of each interrupt priority are
implemented, but for portability priorities should be specified with
integers in the range [0..255].  On Cortex-M4, the top three bits are
implemented.*/

/* set priority for an IRQ to a value [0..255] */
__always_inline__ inline void irq_priority(int irq, uint8_t prio)
{
  prio = prio << 6;
  if (irq < 0)
    SET_BYTE(SCB.SHPR[(irq + 12) >> 2], irq & 0x3, prio);
  else
    SET_BYTE(NVIC.IPR[irq >> 2], irq & 0x3, prio);
}

/* enable interrupts from an IRQ */
__always_inline__ inline void enable_irq(int irq) { NVIC.ISER[0] = BIT(irq); }

/* disable interrupts from a specific IRQ */
__always_inline__ inline void disable_irq(int irq) { NVIC.ICER[0] = BIT(irq); }

/* clear pending interrupt from an IRQ */
__always_inline__ inline void clear_pending(int irq)
{
  NVIC.ICPR[0] = BIT(irq);
}

/* request PendSV interrupt */
__always_inline__ inline void reschedule()
{
  SCB.ICSR = BIT(SCB_ICSR_PENDSVSET);
}

/* active_irq -- find active interrupt: returns -16 to 31 */
__always_inline__ inline uint32_t active_irq()
{
  return GET_FIELD(SCB.ICSR, SCB_ICSR_VECTACTIVE) - 16;
}

__always_inline__ inline void intr_disable() { asm volatile("cpsid i"); }

__always_inline__ inline void intr_enable() { asm volatile("cpsie i"); }

template <typename T>
class intr_guard;

template <>
class intr_guard<void>
{
  int irq;

public:
  intr_guard(void) noexcept : irq(get_primask()) { intr_disable(); }
  ~intr_guard() noexcept { set_primask(irq); }
};

template <>
class intr_guard<int>
{
  int irq;

public:
  intr_guard(int irq) noexcept : irq{irq} { disable_irq(irq); }

  ~intr_guard() noexcept
  {
    clear_pending(irq);
    enable_irq(irq);
  }
};

intr_guard(void) -> intr_guard<void>;
intr_guard(int) -> intr_guard<int>;

__extern_C__ void spin(void);

void delay_loop(uint32_t usecs);
