#include "accel.h"
#include "debug.h"
#include "i2c.h"

/* Different revisions of the micro:bit have different accelerometer chips:
 * - a MMA8653FC on the first revision of V1,
 * - and a LSM303AGR on the second revision of V1 and on V2 (also including the
 *   magnetometer in the same chip).
 * Each chip has its own I2C address and its own internal structure.
 * */

#define ACC 0x1d           /* I2C address of accelerometer */
#define ACC_CTRL_REG1 0x2a /* Control register */
#define ACC_X_DATA 0x01    /* Acceleration data */

namespace accel
{

void read(int *x, int *y, int *z)
{
  int8_t buf[3];
  i2c::read_bytes(ACC, ACC_X_DATA, (uint8_t *)buf, 3);
  *x = -buf[0];
  *y = buf[1];
  *z = -buf[2];
}

void init(void)
{
  /* Find chip and set to 50Hz, 8 bit, Active */
  if (i2c::probe(ACC) == I2C_OK)
    i2c::write_reg(ACC, ACC_CTRL_REG1, 0x23);
  else
    debug<FATAL>("Can't find accelerometer");
}

} // namespace accel
