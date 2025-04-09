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

SEC_ADDR(data);
SEC_ADDR(bss);

SEC_ADDR(startup);
SEC_ADDR(procs);
SEC_ADDR(apps);

__extern_C__ void __reset(void)
{
  // Activate the crystal clock
  CLOCK.HFCLKSTARTED = 0;
  CLOCK.HFCLKSTART = 1;
  while (!CLOCK.HFCLKSTARTED)
    ;

  // protect the first half of the flash
  MPU.PROTENSET0 = 0xFFFFFFFF;

  SEC_INIT(data);
  SEC_ZERO(bss);

  SEC_INIT(startup);
  SEC_INIT(procs);
  SEC_INIT(apps);

  led::init();
  serial::init();
  clear_screen();

  i2c::init();
  fs::init();

  boot::init();

  recover::init();
  tests::run();
  sched::init();

  // should never get here
  trigger_hardfault();
}

