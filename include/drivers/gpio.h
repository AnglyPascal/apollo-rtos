#pragma once

#include "core/hardware.h"

/* GPIO CONVENIENCE */

/* gpio_dir -- set GPIO direction */
inline void gpio_dir(uint32_t pin, uint32_t dir)
{
  if (dir)
    GPIO.DIRSET = BIT(pin);
  else
    GPIO.DIRCLR = BIT(pin);
}

/* gpio_connect -- connect pin for input */
inline void gpio_connect(uint32_t pin)
{
  SET_FIELD(GPIO.PINCNF[pin], GPIO_PINCNF_INPUT, GPIO_INPUT_Connect);
}

/* gpio_drive -- set GPIO drive strength */
inline void gpio_drive(uint32_t pin, uint32_t mode)
{
  SET_FIELD(GPIO.PINCNF[pin], GPIO_PINCNF_DRIVE, mode);
}

/* gpio_out -- set GPIO output value */
inline void gpio_out(uint32_t pin, uint32_t value)
{
  if (value)
    GPIO.OUTSET = BIT(pin);
  else
    GPIO.OUTCLR = BIT(pin);
}

/* gpio_in -- get GPIO input bit */
inline uint32_t gpio_in(uint32_t pin) { return GET_BIT(GPIO.IN, pin); }

