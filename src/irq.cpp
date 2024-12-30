#include "hardware.h"
#include "irq.h"

/* NVIC SETUP FUNCTIONS */

/* On Cortex-M0, only the top two bits of each interrupt priority are
implemented, but for portability priorities should be specified with
integers in the range [0..255].  On Cortex-M4, the top three bits are
implemented.*/

/* irq_priority -- set priority for an IRQ to a value [0..255] */
void irq_priority(int irq, uint32_t prio)
{
  if (irq < 0)
    SET_BYTE(SCB.SHPR[(irq + 12) >> 2], irq & 0x3, prio);
  else
    SET_BYTE(NVIC.IPR[irq >> 2], irq & 0x3, prio);
}

/* enable_irq -- enable interrupts from an IRQ */
void enable_irq(int irq)
{
  NVIC.ISER[0] = BIT(irq);
}

/* disable_irq -- disable interrupts from a specific IRQ */
void disable_irq(int irq)
{
  NVIC.ICER[0] = BIT(irq);
}

/* clear_pending -- clear pending interrupt from an IRQ */
void clear_pending(int irq)
{
  NVIC.ICPR[0] = BIT(irq);
}
