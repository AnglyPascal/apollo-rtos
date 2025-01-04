#include "serial.h"
#include "types.h"

extern "C" int _write(int file, const void *ptr, size_t len)
{
  serial::puts((const char *)ptr, len);

  // return the number of bytes passed (all of them)
  return len;
}
