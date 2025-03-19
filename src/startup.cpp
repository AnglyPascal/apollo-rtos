#include "core/boot.h"
#include "core/hardware.h"
#include "core/memory.h"
#include "core/sched.h"
#include "drivers/display.h"
#include "drivers/i2c.h"
#include "drivers/serial.h"
#include "fs/fs.h"
#include "utility/debug.h"

__extern_C__ byte_t __data_start[], __data_end[], __bss_start[], __bss_end[],
    __end[], __etext[], __stack[], __stack_limit;

__extern_C__ byte_t __startup_load[], __startup_start[], __startup_end[];

__extern_C__ byte_t __procs_load[], __procs_start[], __procs_end[];

__extern_C__ byte_t __apps_load[], __apps_start[], __apps_end[];

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

__extern_C__ void __reset(void)
{
  // Activate the crystal clock
  CLOCK.HFCLKSTARTED = 0;
  CLOCK.HFCLKSTART = 1;
  while (!CLOCK.HFCLKSTARTED)
    ;

  // protect the first half of the flash
  MPU.PROTENSET0 = 0xFFFFFFFF;

  _memcpy(__data_start, __etext, __data_end - __data_start);
  _memset(__bss_start, 0, __bss_end - __bss_start);

  _memcpy(__startup_start, __startup_load, __startup_end - __startup_start);
  _memcpy(__procs_start, __procs_load, __procs_end - __procs_start);
  _memcpy(__apps_start, __apps_load, __apps_end - __apps_start);

  debug_addr<TRACE>();

  led::init();
  serial::init();
  clear_screen();

  i2c::init();
  fs::init();

  boot::init();
  sched::init();

  // should never get here
  trigger_hardfault();
}

