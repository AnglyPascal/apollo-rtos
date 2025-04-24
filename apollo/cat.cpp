#include "core/types.h"
#include "drivers/serial.h"
#include "fs/fs.h"
#include "utility/args.h"
#include "utility/iostream.h"

namespace
{
APP(cat, MID4, 128, param)
{
  auto buf = (args_t *)param;
  auto str = buf->str;

  fn_t fn = (*str == '\0') ? 10 : atoi(str);
  ifstream it{fn};

  while (*it != '\0') {
    printf("%c", *it);
    ++it;
  }
  printf("\r\n");
}
} // namespace

