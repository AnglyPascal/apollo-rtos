/* display.c */
/* Copyright (c) 2020 J. M. Spivey */

#include "display.h"
#include "memory.h"
#include "sched.h"

/* A simple driver for the micro:bit LEDs with the same interface on
V1 and V2. */

void delay_loop(uint32_t);
namespace display
{

namespace
{

/* Note that blank is not the same as an image that is all zeroes, because it
 * has the row bits set.  Copying blank and then setting (actually, clearing)
 * column bits for each row results in an image that displays properly. */
constexpr image_t blank = IMAGE(0, 0, 0, 0, 0, //
                                0, 0, 0, 0, 0, //
                                0, 0, 0, 0, 0, //
                                0, 0, 0, 0, 0, //
                                0, 0, 0, 0, 0);

void image_clear(image_t img)
{
  _memcpy(img, blank, sizeof(image_t));
}

/* encode a pair of integers in one integer */
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

/* find logical row and column for a pixel */
static uint32_t map_pixel(int x, int y)
{
  return img_map[y][x];
}

} // namespace

/* switch on a single pixel in an image */
void image_set(int x, int y, image_t img)
{
  if (x < 0 || x >= 5 || y < 0 || y >= 5)
    return;
  auto p = map_pixel(x, y);
  CLR_BIT(img[XPART(p)], YPART(p));
}

namespace
{
/* image is a shared variable between the client and the display driver task,
 * but it is read-only in the task.  Partial updates don't really matter,
 * because they will cause only a momentary glitch in the display when it is
 * changing anyway. */
image_t image;
} // namespace

/* device driver for LED display */
void *task(void *)
{
  GPIO.DIR = LED_MASK;
  image_clear(image);

  while (1) {
    /* Carefully change LED bits and leave other bits alone */
    int n = 0;
    while (n < 3) {
      GPIO.OUT = image[n++];
      delay_loop(5000);
    }
    sched::sleep(10);
  }

  return nullptr;
}

/* set display from image */
void show(const image_t img)
{
  _memcpy(image, img, sizeof(image_t));
}

/* start display driver task */
void init(void)
{
  sched::reg_proc("display", 2, 128, task, nullptr);
}

void reset()
{
  show(blank);
}

} // namespace display

