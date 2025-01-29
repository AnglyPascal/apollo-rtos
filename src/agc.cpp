#include "accel.h"
#include "debug.h"
#include "display.h"
#include "fs.h"
#include "hardware.h"
#include "i2c.h"
#include "irq.h"
#include "recover.h"
#include "sched.h"
#include "serial.h"
#include "shell.h"
#include "timer.h"

__extern_C__
byte_t __data_start[],
    __data_end[], __bss_start[], __bss_end[], __end[], __etext[], __stack[],
    __stack_limit, __nvm_end[], __nvm_start[];

__extern_C__
byte_t __recover_load[],
    __recover_start[], __recover_end[];

inline void debug_addr()
{
  debug<TRACE>("\tetext:      %x\r\n", __etext);
  debug<TRACE>("\tdata_start: %x\r\n", __data_start);
  debug<TRACE>("\tdata_end:   %x\r\n", __data_end);

  debug<TRACE>("\tend:        %x\r\n", __end);
  debug<TRACE>("\tstack:      %x\r\n", __stack);

  debug<TRACE>("\tbss_start:  %x\r\n", __bss_start);
  debug<TRACE>("\tbss_end:    %x\r\n", __bss_end);
  debug<TRACE>("\tnvm_start:  %x\r\n", __nvm_start);
  debug<TRACE>("\tnvm_end:    %x\r\n", __nvm_end);
}

bool is_reset();
void set_boot();

inline void __start(void)
{
  _memcpy(__data_start, __etext, __data_end - __data_start);
  _memset(__bss_start, 0, __bss_end - __bss_start);

  led::init();
  serial::init();

  fs::mount();
  timer::init();

  i2c::init();
  shell::init();

  serial::clear_screen();

  debug_addr();

  if (!is_reset()) {
    kprintf("boot\r\n");

    // initialize recovery section
    _memcpy(__recover_start, __recover_load, __recover_end - __recover_start);

    // set boot value to true
    set_boot();
  } else {
    kprintf("reset\r\n");
  }

  sched::init();

  spin();
}

__extern_C__
void __reset(void)
{
  /* Activate the crystal clock */
  CLOCK.HFCLKSTARTED = 0;
  CLOCK.HFCLKSTART = 1;
  while (!CLOCK.HFCLKSTARTED)
    ;

  __start();
}

