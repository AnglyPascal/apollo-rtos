#include "fs/fs.h"
#include "core/shell.h"
#include "core/types.h"
#include "utility/debug.h"

namespace shell
{

namespace
{

void rm(void *param)
{
  auto buf = (args_t *)param;
  auto str = buf->str;

  if (*str == '\0')
    return debug<ERROR>("a file name is required\r\n");

  fn_t fn = atoi(str);
  fram::remove(fn);
}

void ls(void *) { fs::trace(); }

void cat(void *param)
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

proc_def_t rm_cmd = {"rm", MID4, 128, rm};
proc_def_t ls_cmd = {"ls", MID4, 128, ls};
proc_def_t cat_cmd = {"cat", MID4, 128, cat};

} // namespace shell
