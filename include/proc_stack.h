#pragma once

#include "allocator.h"
#include "debug.h"
#include "memory.h"
#include "procs.h"
#include "types.h"

class stack_t
{
  // stack layout context for interrupt frames
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

  // Thumb bit is set
  static constexpr uint32_t init_psr = 0x01000000;

  // magic return address for exceptions
  static constexpr uint32_t lr_intr_magic = 0xfffffff9; // not 0xfffffffd

  // minimum stack depth to allow for a printf to execute in debug mode
  static constexpr size_t debug_depth = (size_t)ASSERT_EN * 128;

  static constexpr uint32_t alignment = 16;

  allocator<alloc_stack, alignment> pool;

public:
  // FIXME: set a minimum stack size
  inline void acquire(proc_t *proc, size_t stk_sz, runnable_t func, void *param,
                      void (*ret)(void *))
  {
    stk_sz = roundup(stk_sz, alignment);
    stk_sz += debug_depth;

    proc->stack = pool.alloc(stk_sz + sizeof(context_t));

    if (proc->stack == nullptr) {
      debug<FATAL>("!! STACK NULLPTR\r\n");
      return;
    }

    auto stk_ptr = (context_t *)(proc->stack + stk_sz);

    // setup initial stack frame
    *stk_ptr = {0};
    stk_ptr->psr = init_psr;
    stk_ptr->pc = (uint32_t)func;
    stk_ptr->lr = (uint32_t)ret;
    stk_ptr->r0 = (uint32_t)param;
    stk_ptr->lr_intr = lr_intr_magic;

    proc->stk_ptr = (uint8_t *)stk_ptr;
    proc->stk_sz = stk_sz;
  }

  inline void release(proc_t *proc)
  {
    pool.dealloc(proc->stack);
  }
};

