#pragma once

#include "core/hardware.h"

#define NIMG 3
using image_t = uint32_t[NIMG];

// image constants
#define _ROW(r, c1, c2, c3, c4, c5, c6, c7, c8, c9)                            \
  (BIT(r) | (!c1 << 4) | (!c2 << 5) | (!c3 << 6) | (!c4 << 7) | (!c5 << 8) |   \
   (!c6 << 9) | (!c7 << 10) | (!c8 << 11) | (!c9 << 12))

#define IMAGE(x11, x24, x12, x25, x13, x34, x35, x36, x37, x38, x22, x19, x23, \
              x39, x21, x18, x17, x16, x15, x14, x33, x27, x31, x26, x32)      \
  {_ROW(ROW1, x11, x12, x13, x14, x15, x16, x17, x18, x19),                    \
   _ROW(ROW2, x21, x22, x23, x24, x25, x26, x27, 0, 0),                        \
   _ROW(ROW3, x31, x32, x33, x34, x35, x36, x37, x38, x39)}

#define LED_MASK 0xfff0

namespace led
{
inline void init() { GPIO.DIRSET = LED_MASK; }
inline void dot() { GPIO.OUTSET = 0x5fbf; }
inline void off() { GPIO.OUTCLR = LED_MASK; }
} // namespace led

namespace display
{
void show(const image_t);
void image_set(int x, int y, image_t img);
void reset();
void init();
} // namespace display
