#pragma once

#include "hardware.h"
#include "lib.h"
#include "types.h"

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

void clear_screen();

using listener_t = bool (*)(char);

void register_listener(listener_t);
void unregister_listener();

class listener_guard
{
public:
  listener_guard(listener_t listener)
  {
    register_listener(listener);
  }

  ~listener_guard()
  {
    unregister_listener();
  }
};

} // namespace serial

using serial::printf;
