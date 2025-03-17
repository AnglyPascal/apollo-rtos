#include "fs/fs.h"
#include "core/types.h"
#include "utility/args.h"
#include "utility/debug.h"

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

APP(ls, MID4, 128, param) { fs::trace(); }

APP(cat, MID4, 128, param)
{
  auto str = ((args_t *)param)->str;
  if (*str == '\0')
    return debug<ERROR>("a file name is required\r\n");

  fn_t fn = atoi(str);
  auto file = fram::open(fn, 0, O_READ);

  if (file.ft() != CHAR)
    return debug<ERROR>("only char files allowed\r\n");
}
} // namespace
