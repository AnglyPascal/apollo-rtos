#include "debug.h"
#include "display.h"
#include "fs.h"
#include "hardware.h"
#include "irq.h"
#include "lib.h"
#include "recover.h"
#include "sched.h"
#include "serial.h"
#include "shell.h"
#include "timer.h"
#include "waitlist.h"

__extern_C__
uint8_t __data_start[],
    __data_end[], __bss_start[], __bss_end[], __end[], __etext[], __stack[],
    __stack_limit, __nvm_end[], __nvm_start[];

void waitlist1(void *)
{
  /* led_dot(); */
  waitlist::reg("waitlist1", 640, waitlist1);
}

void waitlist2(void *)
{
  /* led_off(); */
  waitlist::reg("waitlist2", 800, waitlist2);
}

inline void debug_addr()
{
  debug<DEBUG>("\tetext:      %x\r\n", (uint32_t)__etext);
  debug<DEBUG>("\tdata_start: %x\r\n", (uint32_t)__data_start);
  debug<DEBUG>("\tdata_end:   %x\r\n", (uint32_t)__data_end);

  debug<DEBUG>("\tend:        %x\r\n", (uint32_t)__end);
  debug<DEBUG>("\tstack:      %x\r\n", (uint32_t)__stack);

  debug<DEBUG>("\tbss_start:  %x\r\n", (uint32_t)__bss_start);
  debug<DEBUG>("\tbss_end:    %x\r\n", (uint32_t)__bss_end);
  debug<DEBUG>("\tnvm_start:  %x\r\n", (uint32_t)__nvm_start);
  debug<DEBUG>("\tnvm_end:    %x\r\n", (uint32_t)__nvm_end);
}

__extern_C__
void spin(void);

__extern_C__
void trigger_reset(void);

__extern_C__
void __start(void)
{
  _memcpy(__data_start, __etext, __data_end - __data_start);
  _memset(__bss_start, 0, __bss_end - __bss_start);

  led_init();
  serial::init();
  fs::mount(is_reset());

  /* debug_addr(); */

  waitlist::reg("waitlist1", 640, waitlist1);
  waitlist::reg("waitlist2", 800, waitlist2);

  timer::init();
  shell::init();

  if (!is_reset()) {
    printf("boot\r\n");

    waitlist::reg("reset", 10000, [](void *) {
      trigger_reset();
    });
  } else {
    printf("reset\r\n");
  }

  sched::init();

  spin();
}

__extern_C__
void hardfault_handler(void)
{
  debug<FATAL>("!!WTF!!\r\n");

  volatile uint32_t *fault_stack = (uint32_t *)get_msp();
  uint32_t pc = fault_stack[6]; // Program Counter
  uint32_t lr = fault_stack[5]; // Link Register

  debug<FATAL>("pc: %x, lr: %x\r\n", pc, lr);

  serial::flush();
  spin();
  /* trigger_reset(); */
}
