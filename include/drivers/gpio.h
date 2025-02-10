#pragma once

#include "core/hardware.h"

/* GPIO CONVENIENCE */

namespace gpio
{
/* set GPIO direction */
inline void dir(uint32_t pin, uint32_t dir)
{
  if (dir)
    GPIO.DIRSET = BIT(pin);
  else
    GPIO.DIRCLR = BIT(pin);
}

/* connect pin for input */
inline void connect(uint32_t pin)
{
  SET_FIELD(GPIO.PINCNF[pin], GPIO_PINCNF_INPUT, GPIO_INPUT_Connect);
}

/* set GPIO drive strength */
inline void drive(uint32_t pin, uint32_t mode)
{
  SET_FIELD(GPIO.PINCNF[pin], GPIO_PINCNF_DRIVE, mode);
}

/* set GPIO output value */
inline void out(uint32_t pin, uint32_t value)
{
  if (value)
    GPIO.OUTSET = BIT(pin);
  else
    GPIO.OUTCLR = BIT(pin);
}

/* get GPIO input bit */
inline uint32_t in(uint32_t pin) { return GET_BIT(GPIO.IN, pin); }

} // namespace gpio

