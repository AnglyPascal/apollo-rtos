/* display.c */
/* Copyright (c) 2020 J. M. Spivey */

#include "drivers/display.h"

#include "core/memory.h"
#include "core/recover.h"
#include "core/sched.h"

namespace display
{

namespace
{
#define PAIR(x, y) (((x) << 5) | (y))
#define XPART(p) ((p) >> 5)
#define YPART(p) ((p) & 0x1f)

#define PIX(i, j) PAIR(i - 1, j + 3)

/* map of logical row and column bits for each physical pixel */
static uint32_t img_map[5][5] = {
    {PIX(3, 3), PIX(2, 7), PIX(3, 1), PIX(2, 6), PIX(3, 2)},
    {PIX(1, 8), PIX(1, 7), PIX(1, 6), PIX(1, 5), PIX(1, 4)},
    {PIX(2, 2), PIX(1, 9), PIX(2, 3), PIX(3, 9), PIX(2, 1)},
    {PIX(3, 4), PIX(3, 5), PIX(3, 6), PIX(3, 7), PIX(3, 8)},
    {PIX(1, 1), PIX(2, 4), PIX(1, 2), PIX(2, 5), PIX(1, 3)}};

static uint32_t map_pixel(int x, int y) { return img_map[y][x]; }

constexpr image_t blank = IMAGE(0, 0, 0, 0, 0, //
                                0, 0, 0, 0, 0, //
                                0, 0, 0, 0, 0, //
                                0, 0, 0, 0, 0, //
                                0, 0, 0, 0, 0);
image_t image;

} // namespace

/* switch on a single pixel in an image */
void image_set(int x, int y, image_t img)
{
  if (x < 0 || x >= 5 || y < 0 || y >= 5)
    return;
  auto p = map_pixel(x, y);
  CLR_BIT(img[XPART(p)], YPART(p));
}

/* switch on a single pixel in the displayed image */
void image_set(int x, int y)
{
  image_set(x, y, image);
}

void show(const image_t img) { memcpy(image, img, sizeof(image_t)); }

void reset() { show(blank); }

namespace
{
SERVICE(display, LOW2, 128, param)
{
  GPIO.DIR = LED_MASK;
  display::reset();

  while (true) {
    int n = 0;
    while (n < 3) {
      GPIO.OUT = image[n++];
      delay_loop(5000);
    }
    sched::sleep(10);
  }
}
} // namespace
  
} // namespace display
