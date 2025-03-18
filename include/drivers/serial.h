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

void getline(const char *prompt, char *buf, int nbuf);

using listener_t = bool (*)(char);

void register_listener(listener_t);
void unregister_listener();

class listener_guard
{
  bool do_listen;

public:
  listener_guard(listener_t listener, bool do_listen = true)
      : do_listen(do_listen)
  {
    if (do_listen)
      register_listener(listener);
  }

  ~listener_guard()
  {
    if (do_listen)
      unregister_listener();
  }
};

} // namespace serial
