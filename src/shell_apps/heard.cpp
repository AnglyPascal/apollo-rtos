#include "serial.h"
#include "shell.h"
#include "types.h"

namespace shell
{

namespace
{

void heart(void *param)
{
  /* auto str = (const char *)param; */
  /* serial::printf("heart: %s\n", str); */
}

} // namespace

cmd_t heart_cmd = {"heart", 3, 68, heart};

} // namespace shell
