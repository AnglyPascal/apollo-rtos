#include "debug.h"
#include "display.h"
#include "flash.h"
#include "hardware.h"
#include "irq.h"
#include "lib.h"
#include "sched.h"
#include "serial.h"
#include "shell.h"
#include "timer.h"
#include "waitlist.h"

__extern_C__
uint8_t __data_start[],
    __data_end[], __bss_start[], __bss_end[], __end[], __etext[], __stack[],
    __stack_limit;

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

namespace sched
{

namespace
{
void *proc1(void *param)
{
  int n = 5;
  while (n-- > 0)
    ;
  return param;
}

void *proc2(void *param)
{
  int n = 3;
  while (n-- > 0)
    ;
  sched::decr_priority(5);

  while (1) {
    int m = 10;
    while (m-- > 0) {
      n = 100000000;
      while (n-- > 0)
        ;
    }
    sched::sleep(50);
  }

  return param;
}
} // namespace

void setup_procs(void)
{
  sched::reg_proc("proc1", 7, 0, proc1, nullptr);
  sched::reg_proc("proc2", 10, 64, proc2, nullptr);
  display::init();
}

} // namespace sched

inline void debug_addr()
{
  debug<DEBUG>("\tdata_start: %x\r\n", (uint32_t)__data_start);
  debug<DEBUG>("\tdata_end:   %x\r\n", (uint32_t)__data_end);
  debug<DEBUG>("\tbss_start:  %x\r\n", (uint32_t)__bss_start);
  debug<DEBUG>("\tbss_end:    %x\r\n", (uint32_t)__bss_end);
  debug<DEBUG>("\tetext:      %x\r\n", (uint32_t)__etext);
  debug<DEBUG>("\tend:        %x\r\n", (uint32_t)__end);
  debug<DEBUG>("\tstack:      %x\r\n", (uint32_t)__stack);
}

__extern_C__
void spin(void);

__extern_C__
void __start(void)
{
  led_init();
  serial::init();

  /* auto addr = 0x2000; */
  /* uint32_t val = 0xBEEFDEAD; */

  /* auto ptr = (uint32_t *)addr; */
  /* printf("%x, %x\r\n", ptr, *ptr); */
  /* flash::erase(ptr); */
  /* flash::write(ptr, &val, 1); */

  debug_addr();

  waitlist::reg("waitlist1", 640, waitlist1);
  waitlist::reg("waitlist2", 800, waitlist2);

  timer::init();
  shell::init();
  /* display::init(); */
  sched::init();

  spin();
}

__extern_C__
void hardfault_handler(void)
{
  debug<FATAL>("\r\n!!WTF!!\r\n");

  volatile uint32_t *fault_stack = (uint32_t *)get_msp();
  uint32_t pc = fault_stack[6]; // Program Counter
  uint32_t lr = fault_stack[5]; // Link Register

  debug<FATAL>("pc: %x, lr: %x\r\n", pc, lr);

  spin();
}
