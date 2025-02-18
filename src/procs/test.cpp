#include "core/irq.h"
#include "core/recover.h"
#include "core/sched.h"
#include "drivers/fram.h"
#include "drivers/i2c.h"

namespace procs
{

void test_proc(void *param);

proc_def_t test{"test_proc", MID1, 64, test_proc};

void test_proc(void *param)
{
  recover::set_rec(rec_lev_t::RESET);

  // FIXME: move from here
  i2c::scan();

  char str[16] = {'\0'};
  fram::read(0x0, (uint8_t *)str, 16);
  kprintf("fram read: %s\r\n", str);

  int i = 0;
  auto s = "It is a string";
  while (*s != '\0')
    str[i++] = *s++;
  str[i] = '\0';

  fram::write(0x0, (uint8_t *)str, 16);

  delay_loop(1000);
  sched::decr_priority(LOW4);

  while (1) {
    delay_loop(10000);
    sched::sleep(50);
  }
}

} // namespace procs
