/* ubit-v1/startup.c */
/* Copyright (c) 2018 J. M. Spivey */

#include "hardware.h"
#include "irq.h"
#include "memory.h"

/* init -- main program, creates application processes */
void init(void);

__extern_C__
void default_start(void)
{
  /* init(); */
  while (1)
    pause(); /* Halt if init() returns */
}

__extern_C__
void __start(void) __attribute((weak, alias("default_start")));

/* Addresses set by the linker */
__extern_C__
uint8_t __data_start[],
    __data_end[], __bss_start[], __bss_end[], __etext[], __stack[];

/* __reset -- the system starts here */
__extern_C__
void __reset(void)
{
  /* Activate the crystal clock */
  CLOCK.HFCLKSTARTED = 0;
  CLOCK.HFCLKSTART = 1;
  while (!CLOCK.HFCLKSTARTED)
    ;

  int data_size = __data_end - __data_start;
  int bss_size = __bss_end - __bss_start;
  _memcpy(__data_start, __etext, data_size);
  _memset(__bss_start, 0, bss_size);

  __start();
}

/* DEVICE TABLES */

volatile timer_t *const TIMER[] = {&TIMER0, &TIMER1, &TIMER2};

volatile i2c_t *const I2C[] = {&I2C0, &I2C1};

volatile spi_t *const SPI[] = {&SPI0, &SPI1};

// Default interrupt handler

void delay_loop(uint32_t usecs)
{
  uint32_t t = usecs << 1;
  while (t > 0) {
    /* 500nsec per iteration at 16MHz */
    nop();
    nop();
    nop();
    t--;
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
