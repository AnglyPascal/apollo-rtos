#pragma once

#include "hardware.h"
#include <cstdint>

void irq_priority(int irq, uint32_t prio);
void enable_irq(int irq);
void disable_irq(int irq);
void clear_pending(int irq);

/* reschedule -- request PendSV interrupt */
#define reschedule() SCB.ICSR = BIT(SCB_ICSR_PENDSVSET)

/* active_irq -- find active interrupt: returns -16 to 31 */
#define active_irq() (GET_FIELD(SCB.ICSR, SCB_ICSR_VECTACTIVE) - 16)
