#include "lib.h"
#include "types.h"

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
    n = -n;
  }
  utoa(n, p);
}

void btoa(uint8_t n, char *p)
{
  p[10] = '\0';
  p[0] = '0';
  p[1] = 'b';
  for (int i = 9; i > 1; i--) {
    p[i] = '0' + (n & 1);
    n >>= 1;
  }
}

void xtoa(uint32_t n, char *p)
{
  const char *hex = "0123456789abcdef";

  *p++ = '0';
  *p++ = 'x';

  auto s = p;
  p += 8;

  while (p > s) {
    *--p = hex[n & 15];
    n >>= 4;
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

      case 'b': {
        uint8_t n = (uint8_t)va_arg(args, uint32_t);
        auto p = int_buff;
        btoa(n, p);
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

      case 'p': {
        uint32_t n = va_arg(args, uint32_t);
        auto p = int_buff;
        xtoa(n, p);
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
