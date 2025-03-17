#include "drivers/accel.h"

#include "drivers/i2c.h"
#include "utility/debug.h"

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

data_t read()
{
  int8_t buf[3] = {ACC_X_DATA};
  i2c::read_bytes(ACC, (uint8_t *)buf, 1, (uint8_t *)buf, 3);
  return {static_cast<int8_t>(-buf[0]), buf[1], static_cast<int8_t>(-buf[2])};
}

void init(void)
{
  /* Find chip and set to 50Hz, 8 bit, Active */
  assert(i2c::probe(ACC) == I2C_OK, H_RESET);

  uint8_t cmd = ACC_CTRL_REG1;
  i2c::write_reg(ACC, &cmd, 1, 0x23);
}

} // namespace accel
