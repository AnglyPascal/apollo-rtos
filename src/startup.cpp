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

__extern_C__ bool __has_tests;

SECTION_ADDR(data);
SECTION_ADDR(bss);

SECTION_ADDR(startup);
SECTION_ADDR(procs);
SECTION_ADDR(apps);

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
  SECTION_ZERO(bss);

  SECTION_INIT(startup);
  SECTION_INIT(procs);
  SECTION_INIT(apps);

  led::init();
  serial::init();
  clear_screen();

  i2c::init();
  fs::init();

  boot::init();
  recover::init();

  if (__has_tests)
    tests::run();
  else
    boot::stat();

  sched::init();

  // should never get here
  trigger_hardfault();
}

