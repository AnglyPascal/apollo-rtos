#pragma once

#include "lib.h"
#include "types.h"

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

} // namespace serial

/* #define STDIN_FILENO 0  /1* standard input file descriptor *1/ */
/* #define STDOUT_FILENO 1 /1* standard output file descriptor *1/ */
/* #define STDERR_FILENO 2 /1* standard error file descriptor *1/ */

/* #include <cstdio> */
/* using std::printf; */
using serial::printf;
