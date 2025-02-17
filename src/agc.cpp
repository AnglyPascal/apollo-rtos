#include "core/fs.h"
#include "core/hardware.h"
#include "core/memory.h"
#include "core/recover.h"
#include "core/sched.h"
#include "core/shell.h"
#include "drivers/display.h"
#include "drivers/i2c.h"
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "utility/debug.h"

#include "utility/bitset.h"

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

struct flash_t {
  uint32_t magic;
  uint32_t n_reset;
};

enum {
  MAGIC = 0xDEADDEAD,
};

inline void fs_test()
{
  auto file = flash::open(1, sizeof(flash_t), O_CREATE | O_WRITE);
  auto bt = flash::mmap<flash_t>(file);
  if (bt->magic != MAGIC) {
    bt->magic = MAGIC;
    bt->n_reset = 0;
  } else {
    bt->n_reset++;
  }
  kprintf("n_reset: %d\r\n", bt->n_reset);
  flash::store(file);
  flash::unmap(file);
}

inline void __start(void)
{
  _memcpy(__data_start, __etext, __data_end - __data_start);
  _memset(__bss_start, 0, __bss_end - __bss_start);

  led::init();
  serial::init();
  serial::clear_screen();

  fs::init();
  boot::init();

  i2c::init();

  /* shell::init(); */

  // FIXME:
  /** Without this: no heap problem
   *
   *  flash
   *  alloc heap: 0x2000140c
   *  0x20001418, 0
   *  hello
   *  alloc heap: 0x20001468
   *
   *
   * with this: very heap problem
   *
   *  flash
   *  alloc heap: 0x2000140c
   *  n_reset: 0
   *  alloc heap: 0x20001420
   *  0x2000142c, 0
   *  hello
   */
  fs_test();
  debug_addr<TRACE>();

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

