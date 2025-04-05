#include "core/boot.h"
#include "core/hardware.h"
#include "core/memory.h"
#include "core/sched.h"
#include "core/test.h"
#include "drivers/display.h"
#include "drivers/i2c.h"
#include "drivers/serial.h"
#include "fs/fs.h"
#include "utility/debug.h"

__extern_C__ byte_t __end[], __stack[], __stack_limit;

SECTION_ADDR(data);
SECTION_ADDR(bss);
SECTION_ADDR(recover);

SECTION_ADDR(startup);
SECTION_ADDR(procs);
SECTION_ADDR(apps);
SECTION_ADDR(tests);

template <debug_t debug_lev>
inline void debug_addr()
{
  debug<debug_lev>("\tdata_load:  %x\r\n", __data_load);
  debug<debug_lev>("\tdata_start: %x\r\n", __data_start);
  debug<debug_lev>("\tdata_end:   %x\r\n", __data_end);

  debug<debug_lev>("\tend:        %x\r\n", __end);
  debug<debug_lev>("\tstack:      %x\r\n", __stack);

  debug<debug_lev>("\tbss_start:  %x\r\n", __bss_start);
  debug<debug_lev>("\tbss_end:    %x\r\n", __bss_end);
}

void run_tests()
{
  for (SECTION_ITER(tests, test_t, test)) {
    auto [name, func] = *test;
    auto pass = func();

    if (pass)
      printf("passed: %s\r\n", name);
    else
      printf("failed: %s\r\n", name);
  }
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

  SECTION_INIT(data);
  _memset(__bss_start, 0, __bss_end - __bss_start);

  SECTION_INIT(startup);
  SECTION_INIT(procs);
  SECTION_INIT(apps);
  SECTION_INIT(tests);

  debug_addr<TRACE>();

  led::init();
  serial::init();
  clear_screen();

  i2c::init();
  fs::init();

  boot::init();

  // initialize recovery section
  if (boot::lev() != boot_lev_t::RESET)
    SECTION_INIT(recover);

  run_tests();

  sched::init();

  // should never get here
  trigger_hardfault();
}

