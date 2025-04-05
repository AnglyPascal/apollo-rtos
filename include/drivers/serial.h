#pragma once

#include "core/hardware.h"
#include "core/irq.h"

#define CTRL(x) ((x) & 0x1f)
#define BS (8)
#define DEL (0177)
#define ESC (7)
#define CLEAR_LINE ("\r\033[2K")

namespace serial
{

void init(void);

char getc(void);

void busy_putc(char ch);
void intr_putc(char ch);

inline void (*putc)(char) = intr_putc;

using listener_t = bool (*)(char);

void register_listener(listener_t);
void unregister_listener();

struct listener_guard {
  listener_guard(listener_t listener) { register_listener(listener); }
  ~listener_guard() { unregister_listener(); }
};

} // namespace serial
