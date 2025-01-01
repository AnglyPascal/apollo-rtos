#pragma once

#include "lib.h"
#include "types.h"

namespace serial
{

void init(void);
int getc(void);
void putc(char ch);
void puts(const char *s);
void getline(const char *prompt, char *buf, int nbuf);

template <typename... Args>
void printf(Args... args)
{
  do_printf(putc, args...);
}

} // namespace serial

using serial::printf;
