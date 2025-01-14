#include "serial.h"
#include "types.h"

__extern_C__
int _write(int file, const void *ptr, size_t len)
{
  serial::puts((const char *)ptr, len);

  // return the number of bytes passed (all of them)
  return len;
}

/* #define STDIN_FILENO 0  /1* standard input file descriptor *1/ */
/* #define STDOUT_FILENO 1 /1* standard output file descriptor *1/ */
/* #define STDERR_FILENO 2 /1* standard error file descriptor *1/ */

/* #include <cstdio> */
/* using std::printf; */
