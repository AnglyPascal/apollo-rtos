#include "lib.h"
#include "hardware.h"
#include "types.h"

#include "serial.h"

int32_t atoi(const char *p)
{
  auto atou = [](const char *p) {
    int n = 0;
    while (*p >= '0' && *p <= '9') {
      n = n * 10 + *p++ - '0';
    }
    return n;
  };

  return *p == '-' ? -atou(++p) : atou(p);
}

void utoa(uint32_t n, char *p)
{
  auto s = p;

  do {
    *p++ = '0' + (n % 10);
    n /= 10;
  } while (n > 0);

  *p-- = '\0';

  while (s < p) {
    auto c = *s;
    *s++ = *p;
    *p-- = c;
  }
}

void itoa(int32_t n, char *p)
{
  if (n < 0) {
    *p++ = '-';
    utoa(-n, p);
  } else {
    utoa(n, p);
  }
}

void xtoa(uint32_t n, char *p)
{
  auto s = p;
  do {
    auto c = '0' + (n & 15);
    *p++ = c > '9' ? 'A' + (c - '9' - 1) : c;
    n >>= 4;
  } while (n > 0);

  while (p < s + 8) {
    *p++ = '0';
  }

  *p++ = 'x';
  *p++ = '0';
  *p-- = '\0';

  while (s < p) {
    auto c = *s;
    *s++ = *p;
    *p-- = c;
  }
}

char int_buff[11];

void do_printf(void (*putc)(char), const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  while (*fmt != '\0') {
    if (*fmt == '%') {
      auto c = *(++fmt);

      switch (c) {
      case '\0': {
        putc('%');
        break;
      }

      case 'd': {
        int32_t n = va_arg(args, int32_t);
        auto p = int_buff;
        itoa(n, p);
        while (*p != '\0')
          putc(*p++);
        break;
      }

      case 'u': {
        uint32_t n = va_arg(args, uint32_t);
        auto p = int_buff;
        utoa(n, p);
        while (*p != '\0')
          putc(*p++);
        break;
      }

      case 'x': {
        uint32_t n = va_arg(args, uint32_t);
        auto p = int_buff;
        xtoa(n, p);
        while (*p != '\0')
          putc(*p++);
        break;
      }

      case 's': {
        const char *str = va_arg(args, const char *);
        while (*str != '\0')
          putc(*str++);
        break;
      }

      case 'c': {
        char d = va_arg(args, int);
        putc(d);
        break;
      }

      default: {
        putc('%');
        putc(c);
      }
      }
    } else {
      putc(*fmt);
    }
    fmt++;
  }

  va_end(args);
}

namespace serial
{

} // namespace serial
