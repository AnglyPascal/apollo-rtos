#include "core/sched.h"
#include "core/types.h"
#include "core/waitlist.h"
#include "drivers/serial.h"
#include "fs/fs.h"
#include "utility/debug.h"

struct __attribute__((packed)) fault_stack_t {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t pc;
  uint32_t xpsr;
};

__extern_C__ void hardfault_handler_body(void *fault_stack)
{
  serial::putc = serial::busy_putc;

  auto stk = (fault_stack_t *)fault_stack;

  if (stk->r2 == HARDFAULT_MAGIC) {
    debug<FATAL>(BOLD RED "assertion failure at " //
                 BOLD YELLOW "%s:%d" DEFAULT "\r\n",
                 (const char *)stk->r0, stk->r1);
  } else {
    debug<FATAL>("\r\n" BOLD RED "hardfault" DEFAULT "\r\n" //
                 "pc: " BOLD YELLOW "%x" DEFAULT ", "       //
                 "lr: " BOLD YELLOW "%x" DEFAULT "\r\n"     //
                 ,                                          //
                 stk->pc, stk->lr);
  }

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
      fs::trace();
      break;

    default:
      break;
    }
  }
}

