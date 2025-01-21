#include "debug.h"
#include "fs.h"
#include "sched.h"
#include "serial.h"
#include "types.h"
#include "waitlist.h"

__extern_C__
void trigger_reset(void);

__extern_C__
void hardfault_handler(void)
{
  serial::putc = serial::busy_putc;

  debug<FATAL>("!!! WTF !!!\r\n");

  volatile uint32_t *fault_stack = (uint32_t *)get_msp();
  uint32_t pc = fault_stack[6]; // Program Counter
  uint32_t lr = fault_stack[5]; // Link Register

  debug<FATAL>("pc: %x, lr: %x\r\n", pc, lr);

  while (1) {
    char ch = serial::getc();

    switch (ch) {
    case CTRL('d'):
      trigger_reset();
      break;

    case '?':
      sched::trace();
      waitlist::trace();
      fs::trace();
      break;

    default:
      serial::putc(ch);
    }
  }
}

