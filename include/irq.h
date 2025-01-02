#pragma once

#include "hardware.h"
#include "types.h"

/* NVIC SETUP FUNCTIONS */

/* On Cortex-M0, only the top two bits of each interrupt priority are
implemented, but for portability priorities should be specified with
integers in the range [0..255].  On Cortex-M4, the top three bits are
implemented.*/

/* set priority for an IRQ to a value [0..255] */
__always_inline__
inline void irq_priority(int irq, uint8_t prio)
{
  if (irq < 0)
    SET_BYTE(SCB.SHPR[(irq + 12) >> 2], irq & 0x3, prio);
  else
    SET_BYTE(NVIC.IPR[irq >> 2], irq & 0x3, prio);
}

/* enable interrupts from an IRQ */
__always_inline__
inline void enable_irq(int irq)
{
  NVIC.ISER[0] = BIT(irq);
}

/* disable interrupts from a specific IRQ */
__always_inline__
inline void disable_irq(int irq)
{
  NVIC.ICER[0] = BIT(irq);
}

/* clear pending interrupt from an IRQ */
__always_inline__
inline void clear_pending(int irq)
{
  NVIC.ICPR[0] = BIT(irq);
}

/* request PendSV interrupt */
__always_inline__
inline void reschedule()
{
  SCB.ICSR = BIT(SCB_ICSR_PENDSVSET);
}

/* active_irq -- find active interrupt: returns -16 to 31 */
__always_inline__
inline uint32_t active_irq()
{
  return GET_FIELD(SCB.ICSR, SCB_ICSR_VECTACTIVE) - 16;
}

#define intr_disable() asm volatile("cpsid i")
#define intr_enable() asm volatile("cpsie i")
