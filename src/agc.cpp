#include "flash.h"
#include "hardware.h"
#include "irq.h"
#include "lib.h"
#include "sched.h"
#include "serial.h"
#include "shell.h"
#include "timer.h"
#include "waitlist.h"

extern "C" uint8_t __data_start[], __data_end[], __bss_start[], __bss_end[],
    __end[], __etext[], __stack[], __stack_limit;

void waitlist1(void *)
{
  printf("1: %d \r\r\n", timer::now());
  led_dot();
  waitlist::reg(640, waitlist1);
}

void waitlist2(void *)
{
  led_off();
  printf("2: %d \r\r\n", timer::now());
  waitlist::reg(800, waitlist2);
}

namespace sched
{

namespace
{
void *proc1(void *param)
{
  int n = 5;
  while (n-- > 0) {
    /* serial::puts("\t\t\t1\r\n"); */
  }
  return param;
}

void *proc2(void *param)
{
  int n = 3;
  while (n-- > 0) {
    /* serial::puts("\t\t\t2\r\n"); */
  }
  sched::decr_priority(5);

  while (1) {
    int m = 10;
    while (m-- > 0) {
      /* printf("."); */
      n = 100000000;
      while (n-- > 0)
        ;
    }
    /* printf("\r\n"); */
    sched::sleep(50);
  }

  return param;
}
} // namespace

void setup_procs(void)
{
  sched::reg_proc("proc1", 7, 512, proc1, nullptr);
  sched::reg_proc("proc2", 10, 512, proc2, nullptr);
}

} // namespace sched

extern "C" void __start(void)
{
  led_init();
  serial::init();

  /* auto addr = 0x2000; */
  /* uint32_t val = 0xBEEFDEAD; */

  /* auto ptr = (uint32_t *)addr; */
  /* printf("%x, %x\r\n", ptr, *ptr); */
  /* flash::erase(ptr); */
  /* flash::write(ptr, &val, 1); */

  printf("\tdata_start: %x\r\n", (uint32_t)__data_start);
  printf("\tdata_end:   %x\r\n", (uint32_t)__data_end);
  printf("\tbss_start:  %x\r\n", (uint32_t)__bss_start);
  printf("\tbss_end:    %x\r\n", (uint32_t)__bss_end);
  printf("\tetext:      %x\r\n", (uint32_t)__etext);
  printf("\tend:        %x\r\n", (uint32_t)__end);
  printf("\tstack:      %x\r\n", (uint32_t)__stack);

  /* waitlist::reg(640, waitlist1); */
  /* waitlist::reg(800, waitlist2); */

  timer::init();
  shell::init();
  sched::init();
  while (1)
    ;
}

extern "C" void spin(void);

extern "C" void hardfault_handler(void)
{
  serial::printf("!!WTF!!\n");
  spin();
}
