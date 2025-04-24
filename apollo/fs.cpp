#include "fs/fs.h"
#include "core/types.h"
#include "drivers/serial.h"
#include "utility/args.h"
#include "utility/iostream.h"

namespace
{
APP(rm, MID4, 128, param)
{
  auto buf = (args_t *)param;
  auto str = buf->str;

  if (*str == '\0')
    return debug<ERROR>("a file name is required\r\n");

  fn_t fn = atoi(str);
  fram::remove(fn);
}

APP(ls, MID4, 128, param)
{
  fs::trace();
  printf("\r\n");
}

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
