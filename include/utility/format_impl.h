#include "core/hardware.h"
#include "core/types.h"

#include <cstdarg>

__noinline__ inline int32_t atoi(const char *p)
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

inline void utoa(uint32_t n, char *p)
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

inline void itoa(int32_t n, char *p)
{
  if (n < 0) {
    *p++ = '-';
    n = -n;
  }
  utoa(n, p);
}

inline void btoa(uint8_t n, char *p)
{
  p[10] = '\0';
  p[0] = '0';
  p[1] = 'b';
  for (int i = 9; i > 1; i--) {
    p[i] = '0' + (n & 1);
    n >>= 1;
  }
}

inline void xtoa(uint32_t n, char *p)
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

inline char int_buf[11];
inline char float_buf[6];

inline void rtoa(rational_t r, char *ip, char *fp)
{
  uint32_t mult = rational_t::mult;

  int32_t num = (r.num * 100 * mult) / r.denom;

  if (num < 0) {
    *ip++ = '-';
    num = -num;
  }

  int32_t int_val = num / mult;
  uint32_t frac_val = num % mult;

  utoa(int_val, ip);
  utoa(frac_val, fp);
}

template <typename _ostream>
  requires requires(_ostream &os, char c) {
    { os.putc(c) };
  }
__noinline__ void do_printf(_ostream &os, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  while (*fmt != '\0') {
    if (*fmt == '%') {
      auto c = *(++fmt);

      switch (c) {
      case '\0': {
        os.putc('%');
        break;
      }

      case 'f': {
        rational_t f = va_arg(args, rational_t);

        auto ib = int_buf, fb = float_buf;
        rtoa(f, ib, fb);

        while (*ib != '\0')
          os.putc(*ib++);
        os.putc('.');
        while (*fb != '\0')
          os.putc(*fb++);

        break;
      }

      case 'd': {
        int32_t n = va_arg(args, int32_t);
        auto p = int_buf;
        itoa(n, p);
        while (*p != '\0')
          os.putc(*p++);
        break;
      }

      case 'b': {
        uint8_t n = (uint8_t)va_arg(args, uint32_t);
        auto p = int_buf;
        btoa(n, p);
        while (*p != '\0')
          os.putc(*p++);
        break;
      }

      case 'u': {
        uint32_t n = va_arg(args, uint32_t);
        auto p = int_buf;
        utoa(n, p);
        while (*p != '\0')
          os.putc(*p++);
        break;
      }

      case 'p': {
        uint32_t n = va_arg(args, uint32_t);
        auto p = int_buf;
        xtoa(n, p);
        while (*p != '\0')
          os.putc(*p++);
        break;
      }

      case 'x': {
        uint32_t n = va_arg(args, uint32_t);
        auto p = int_buf;
        xtoa(n, p);
        while (*p != '\0')
          os.putc(*p++);
        break;
      }

      case 's': {
        const char *str = va_arg(args, const char *);
        while (*str != '\0')
          os.putc(*str++);
        break;
      }

      case 'c': {
        char d = va_arg(args, int);
        os.putc(d);
        break;
      }

      default: {
        os.putc('%');
        os.putc(c);
      }
      }
    } else {
      os.putc(*fmt);
    }
    fmt++;
  }

  va_end(args);
}

__noinline__ inline uint8_t rand()
{
  RNG.CONFIG = RNG_CONFIG_DERCEN;

  RNG.START = 1;
  RNG.SHORTS = 1;

  while (!RNG.VALRDY)
    ;
  RNG.VALRDY = 0;

  return (uint8_t)RNG.VALUE;
}

// FIXME: very biased for large numbers
__noinline__ inline uint32_t random()
{
  RNG.CONFIG = RNG_CONFIG_DERCEN;

  RNG.START = 1;

  uint32_t val = 0;
  for (int i = 0; i < 8; i++) {
    while (!RNG.VALRDY)
      ;
    RNG.VALRDY = 0;
    auto temp = RNG.VALUE;
    temp >>= 4;
    val = val << 4 | temp;
  }

  RNG.STOP = 1;

  return val;
}
