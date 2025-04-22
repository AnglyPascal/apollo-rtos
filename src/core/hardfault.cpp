#include "core/sched.h"
#include "core/types.h"
#include "core/waitlist.h"
#include "drivers/serial.h"
#include "fs/fs.h"
#include "utility/debug.h"

struct context_t {
  // saved manually
  uint32_t r8;  // 5
  uint32_t r9;  // 6
  uint32_t r10; // 7
  uint32_t r11; // 8

  uint32_t r4;      // 1
  uint32_t r5;      // 2
  uint32_t r6;      // 3
  uint32_t r7;      // 4
  uint32_t lr_intr; // 0

  // saved by hardware
  uint32_t r0;  // 9
  uint32_t r1;  // 10
  uint32_t r2;  // 11
  uint32_t r3;  // 12
  uint32_t r12; // 13
  uint32_t lr;  // 14
  uint32_t pc;  // 15
  uint32_t psr; // 16
};

__extern_C__ void hardfault_handler_body(void *fault_stack)
{
  serial::os.__putc = serial::busy_putc;
  intr_guard guard{UART_IRQ};

  auto stk = (context_t *)fault_stack;

  if (stk->r2 == HARDFAULT_MAGIC) {
    debug<FATAL>(BOLD RED "assertion failure at " //
                 BOLD YELLOW "%s:%d" DEFAULT "\r\n",
                 (const char *)stk->r0, stk->r1);
  } else {
    debug<FATAL>("\r\n" BOLD RED "hardfault" DEFAULT "\r\n" //
                 "pc: " BOLD YELLOW "%x" DEFAULT "\t"       //
                 "lr: " BOLD YELLOW "%x" DEFAULT "\r\n"     //
                 ,                                          //
                 stk->pc, stk->lr);

    debug<FATAL>(DEFAULT //
                 "r0: " BOLD "%x" DEFAULT "\t"
                 "r4: " BOLD "%x" DEFAULT "\r\n"
                 "r1: " BOLD "%x" DEFAULT "\t"
                 "r5: " BOLD "%x" DEFAULT "\r\n"
                 "r2: " BOLD "%x" DEFAULT "\t"
                 "r6: " BOLD "%x" DEFAULT "\r\n"
                 "r3: " BOLD "%x" DEFAULT "\t"
                 "r7: " BOLD "%x" DEFAULT "\r\n",
                 stk->r0, stk->r1, stk->r2, stk->r3, stk->r4, stk->r5, stk->r6,
                 stk->r7);
  }

  while (1) {
    char ch = serial::getc();
    printf("%c", ch);

    switch (ch) {
    case CTRL('d'):
      trigger_reset();
      break;

    case '?':
      printf("\r\n");
      sched::trace();
      waitlist::trace();
      fs::trace();
      break;

    default:
      break;
    }
  }
}

