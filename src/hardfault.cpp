#include "debug.h"
#include "fs.h"
#include "sched.h"
#include "serial.h"
#include "types.h"
#include "waitlist.h"

__extern_C__
void trigger_reset(void);

__extern_C__
void hardfault_handler_body(uint32_t *fault_stack)
{
  serial::putc = serial::busy_putc;

  uint32_t pc = fault_stack[6];
  uint32_t lr = fault_stack[5];
  debug<FATAL>("hardfault\r\npc: %x, lr: %x\r\n", pc, lr);

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

