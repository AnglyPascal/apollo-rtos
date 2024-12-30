#pragma once

#include "types.h"

namespace serial
{

void init(void);
int getc(void);
void putc(char ch);
void puts(const char *s);
void getline(const char *prompt, char *buf, int nbuf);
void printf(const char *fmt, ...);

} // namespace serial

using serial::printf;
