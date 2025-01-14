#pragma once

#include "lib.h"
#include "types.h"

#define CTRL(x) ((x) & 0x1f)

namespace serial
{

void init(void);
int getc(void);
void putc(char ch);
void puts(const char *s);
void puts(const char *s, size_t len);
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

} // namespace serial

using serial::printf;
