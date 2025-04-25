#include "fs/fs.h"
#include "core/sched.h"
#include "core/types.h"
#include "drivers/serial.h"
#include "utility/args.h"
#include "utility/iostream.h"

namespace
{
APP(rm, MID4, 128, param)
{
  auto args = (args_t *)param;

  auto forced = args->get_option() == 'f';
  if (*args->s == '\0')
    return debug<ERROR>("requires a file name\r\n");

  auto fn = args->get_uint<fn_t>();
  if (fn == MAX<fn_t>)
    return debug<ERROR>("invalid file name\r\n");

  fram::remove(fn, forced);
}

APP(touch, MID4, 128, param)
{
  auto args = (args_t *)param;

  auto perm = args->get_option() == 'p';

  if (*args->s == '\0')
    return debug<ERROR>("requires a file name\r\n");

  auto fn = args->get_uint<fn_t>();
  if (fn == MAX<fn_t>)
    return debug<ERROR>(DEFAULT "invalid file name: %d\r\n", fn);

  auto file = fram::open(fn, O_CREATE | O_CHAR_FILE | (perm ? O_PERM : 0));
  if (!file.new_file)
    return debug<WARN>(DEFAULT "file alrady exists\r\n");
}

APP(ls, MID4, 128, param)
{
  auto args = (args_t *)param;
  auto fn = args->get_uint<fn_t>();
  fs::trace(fn == MAX<fn_t> ? null_fn : fn);
}

APP(cat, MID4, 128, param)
{
  auto buf = (args_t *)param;
  auto str = buf->str;

  fn_t fn = (*str == '\0') ? 10 : atoi(str);
  ifstream it{fn};

  // FIXME: buffer it
  while (*it != '\0') {
    printf("%c", *it);
    ++it;
  }
  printf("\r\n");
}
} // namespace
