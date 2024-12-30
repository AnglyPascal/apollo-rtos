#include "hardware.h"

/* GPIO CONVENIENCE */

/* gpio_dir -- set GPIO direction */
void gpio_dir(uint32_t pin, uint32_t dir)
{
  if (dir)
    GPIO.DIRSET = BIT(pin);
  else
    GPIO.DIRCLR = BIT(pin);
}

/* gpio_connect -- connect pin for input */
void gpio_connect(uint32_t pin)
{
  SET_FIELD(GPIO.PINCNF[pin], GPIO_PINCNF_INPUT, GPIO_INPUT_Connect);
}

/* gpio_drive -- set GPIO drive strength */
void gpio_drive(uint32_t pin, uint32_t mode)
{
  SET_FIELD(GPIO.PINCNF[pin], GPIO_PINCNF_DRIVE, mode);
}

/* gpio_out -- set GPIO output value */
void gpio_out(uint32_t pin, uint32_t value)
{
  if (value)
    GPIO.OUTSET = BIT(pin);
  else
    GPIO.OUTCLR = BIT(pin);
}

/* gpio_in -- get GPIO input bit */
uint32_t gpio_in(uint32_t pin)
{
  return GET_BIT(GPIO.IN, pin);
}
