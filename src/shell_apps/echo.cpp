#include "char_buffer.h"
#include "memory.h"
#include "serial.h"
#include "shell.h"
#include "types.h"

namespace shell
{

namespace
{

void *echo(void *param)
{
  auto buf = (char_buffer<64> *)param;
  printf(">> %s\n", buf->str + 5);
  return param;
}

} // namespace

cmd_t echo_cmd = {"echo", 3, 128, echo};

} // namespace shell
