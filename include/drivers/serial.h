#pragma once

#include "core/hardware.h"
#include "core/irq.h"
#include "core/types.h"
#include "utility/lib.h"

#define CTRL(x) ((x) & 0x1f)

namespace serial
{

void init(void);

char getc(void);

void busy_putc(char ch);
void intr_putc(char ch);

inline void (*putc)(char) = intr_putc;

void getline(const char *prompt, char *buf, int nbuf);

template <typename... Args>
void printf(Args... args)
{
  do_printf(putc, args...);
}

template <typename... Args>
void kprintf(Args... args)
{
  intr_guard guard;
  do_printf(busy_putc, args...);
}

inline void clear_screen() { kprintf("\033[2J\033[H"); }

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

using serial::kprintf;
using serial::printf;
