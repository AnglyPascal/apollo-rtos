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

SEC_ADDR(services);
SEC_ADDR(startups);
SEC_ADDR(apps);

namespace
{
static const char *welcome_mssg =                              //
    BOLD YELLOW                                                //
    "  _____         _ _        _____ _____ _____ _____  \r\n" //
    " |  _  |___ ___| | |___   | __  |_   _|     |   __| \r\n" //
    " |     | . | . | | | . |  |    -| | | |  |  |__   | \r\n" //
    " |__|__|  _|___|_|_|___|  |__|__| |_| |_____|_____| \r\n" //
    "       |_|                                          \r\n" //
    DEFAULT "\r\n"                                             //
    YELLOW " Welcome to Apollo RTOS!\r\n\r\n" DEFAULT;
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

  SEC_INIT(data);
  SEC_ZERO(bss);

  SEC_INIT(services);
  SEC_INIT(startups);
  SEC_INIT(apps);

  mem::init();
  led::init();
  serial::init();

  i2c::init();
  fs::init();

  boot::init();
  if (boot::is_boot()) {
    clear_screen();
    debug<INFO>(welcome_mssg);
  }

  recover::init();
  tests::run();
  sched::init();

  // should never get here
  trigger_hardfault();
}

