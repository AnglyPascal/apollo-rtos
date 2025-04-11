#pragma once

#include "core/memory.h"
#include "core/types.h"
#include "utility/allocator.h"
#include "utility/debug.h"

#include "procs.h"

class stack_allocator_t
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

  // minimum stack depth
  static constexpr size_t min_stack_sz = 128 * 3;

  static constexpr uint32_t alignment = 16;

  allocator<alloc_stack, alignment> pool;

public:
  inline void acquire(proc_t *proc, size_t stk_sz, runnable_t func, void *param,
                      void (*ret)())
  {
    stk_sz = roundup(stk_sz, alignment);
    stk_sz += min_stack_sz;

    proc->stack = (byte_t *)pool.alloc(stk_sz);

    if (proc->stack == nullptr) {
      debug<FATAL>("!! STACK NULLPTR\r\n");
      return;
    }

    auto stk_ptr = (context_t *)(proc->stack + stk_sz - sizeof(context_t));

    // setup initial stack frame
    *stk_ptr = {0};
    stk_ptr->psr = init_psr;
    stk_ptr->pc = (uint32_t)func;
    stk_ptr->lr = (uint32_t)ret;
    stk_ptr->r0 = (uint32_t)param;
    stk_ptr->lr_intr = lr_intr_magic;

    proc->stk_ptr = (byte_t *)stk_ptr;
    proc->stk_sz = stk_sz;
  }

  void init_list() { return pool.init_list(); }

  inline void release(proc_t *proc) { pool.dealloc(proc->stack); }
};
