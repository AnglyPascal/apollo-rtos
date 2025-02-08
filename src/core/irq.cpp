/* ubit-v1/startup.c */
/* Copyright (c) 2018 J. M. Spivey */

#include "core/irq.h"

#include "core/hardware.h"
#include "core/types.h"

// Default interrupt handler

void delay_loop(uint32_t usecs)
{
  uint32_t t = usecs << 1;
  while (t-- > 0) {
    /* 500nsec per iteration at 16MHz */
    nop();
    nop();
    nop();
  }
}

/* spin -- show Seven Stars of Death */
__extern_C__
void spin(void)
{
  intr_disable();

  GPIO.DIR = 0xfff0;
  while (1) {
    GPIO.OUT = 0x4000;
    delay_loop(500000);
    GPIO.OUT = 0;
    delay_loop(100000);
  }
}

__extern_C__
void default_handler(void) __attribute((weak, alias("spin")));

