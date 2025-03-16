#include "core/boot.h"
#include "core/hardware.h"
#include "core/memory.h"
#include "core/sched.h"
#include "drivers/display.h"
#include "drivers/i2c.h"
#include "drivers/serial.h"
#include "fs/fs.h"
#include "utility/debug.h"

__extern_C__
byte_t __data_start[],
    __data_end[], __bss_start[], __bss_end[], __end[], __etext[], __stack[],
    __stack_limit;

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
}

inline void __start(void)
{
  _memcpy(__data_start, __etext, __data_end - __data_start);
  _memset(__bss_start, 0, __bss_end - __bss_start);

  debug_addr<TRACE>();

  led::init();
  serial::init();
  serial::clear_screen();

  i2c::init();
  fs::init();

  boot::init();
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

