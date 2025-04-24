#pragma once

#include <cstdint>

#include "core/irq.h"
#include "drivers/serial.h"

template <typename _ostream>
  requires requires(_ostream &os, char c) {
    { os.putc(c) };
  }
__noinline__ void do_printf(_ostream &os, const char *fmt, ...);

inline void kputc(char c)
{
  serial::flush();
  intr_guard guard{UART_IRQ};
  return serial::kos.putc(c);
}

template <typename... Args>
void kprintf(Args... args)
{
  serial::flush();
  intr_guard guard{UART_IRQ};
  do_printf(serial::kos, std::forward<Args>(args)...);
}

inline void clear_screen() { kprintf("\033[2J\033[H"); }
inline void clear_line() { kprintf("\r\033[K"); }

int32_t atoi(const char *p, const char **end = nullptr);
uint32_t atou(const char *p, const char **end = nullptr);

// FIXME find a better place
uint8_t rand();
uint32_t random();

#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define LIGHT_BLACK "\033[90m"
#define LIGHT_RED "\033[91m"
#define LIGHT_GREEN "\033[92m"
#define LIGHT_YELLOW "\033[93m"
#define LIGHT_BLUE "\033[94m"
#define LIGHT_MAGENTA "\033[95m"
#define LIGHT_CYAN "\033[96m"
#define LIGHT_WHITE "\033[97m"

#define DEFAULT "\033[0m"
#define BOLD "\033[1m"

#include "format_impl.h"

