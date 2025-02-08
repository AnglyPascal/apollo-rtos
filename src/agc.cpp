#include "core/fs.h"
#include "core/hardware.h"
#include "core/sched.h"
#include "core/shell.h"
#include "drivers/display.h"
#include "drivers/i2c.h"
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "utility/debug.h"

__extern_C__
byte_t __data_start[],
    __data_end[], __bss_start[], __bss_end[], __end[], __etext[], __stack[],
    __stack_limit, __nvm_end[], __nvm_start[];

__extern_C__
byte_t __recover_load[],
    __recover_start[], __recover_end[];

template <debug_t debug_lev>
inline void debug_addr()
{
  debug<debug_lev>("\tetext:      %x\r\n", __etext);
  debug<debug_lev>("\tdata_start: %x\r\n", __data_start);
  debug<debug_lev>("\tdata_end:   %x\r\n", __data_end);

  debug<debug_lev>("\tend:        %x\r\n", __end);
  debug<debug_lev>("\tstack:      %x\r\n", __stack);

  debug<debug_lev>("\tbss_start:  %x\r\n", __bss_start);
  debug<debug_lev>("\tbss_end:    %x\r\n", __bss_end);
  debug<debug_lev>("\tnvm_start:  %x\r\n", __nvm_start);
  debug<debug_lev>("\tnvm_end:    %x\r\n", __nvm_end);
}

bool is_reset();
void set_boot();

inline void __start(void)
{
  _memcpy(__data_start, __etext, __data_end - __data_start);
  _memset(__bss_start, 0, __bss_end - __bss_start);

  led::init();
  serial::init();
  serial::clear_screen();

  fs::mount();
  timer::init();

  i2c::init();
  shell::init();

  debug_addr<TRACE>();

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

