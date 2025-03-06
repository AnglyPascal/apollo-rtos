#include "fs/fs.h"
#include "core/sched.h"
#include "core/types.h"
#include "core/waitlist.h"
#include "drivers/serial.h"
#include "utility/debug.h"

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
    serial::putc(ch);

    switch (ch) {
    case CTRL('d'):
      trigger_reset();
      break;

    case '?':
      kprintf("\r\n");
      sched::trace();
      waitlist::trace();
      /* fs::trace(); */
      break;

    default:
      break;
    }
  }
}

