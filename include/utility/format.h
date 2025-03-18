#pragma once

#include <cstdarg>
#include <cstdint>

#include "core/irq.h"
#include "drivers/serial.h"

void do_printf(void (*putch)(char), const char *fmt, ...);

template <typename... Args>
void printf(Args... args)
{
  do_printf(serial::putc, args...);
}

template <typename... Args>
void kprintf(Args... args)
{
  intr_guard guard;
  do_printf(serial::busy_putc, args...);
}

inline void clear_screen() { kprintf("\033[2J\033[H"); }

int32_t atoi(const char *p);
uint32_t xtou(char *p);

// FIXME find a better place
uint8_t rand();
uint32_t random();
