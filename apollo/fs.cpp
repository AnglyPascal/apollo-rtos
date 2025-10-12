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

  auto fn = args->get_uint<fn_t>().value_or(null_fn);
  if (fn == null_fn)
    return debug<ERROR>("invalid file name\r\n");

  fram::remove(fn, forced);
}

APP(touch, MID4, 128, param)
{
  auto args = (args_t *)param;

  auto perm = args->get_option() == 'p';

  if (*args->s == '\0')
    return debug<ERROR>("requires a file name\r\n");

  auto fn = args->get_uint<fn_t>().value_or(null_fn);
  if (fn == null_fn)
    return debug<ERROR>(DEFAULT "invalid file name: %d\r\n", fn);

  auto file = fram::open(fn, O_CREATE | O_CHAR_FILE | (perm ? O_PERM : 0));
  if (!file.new_file)
    return debug<WARN>(DEFAULT "file alrady exists\r\n");
}

APP(ls, MID4, 128, param)
{
  auto args = (args_t *)param;
  auto fn = args->get_uint<fn_t>().value_or(null_fn);
  fs::trace(fn);
}

APP(cat, MID4, 128, param)
{
  auto buf = (args_t *)param;
  auto str = buf->str;

  fn_t in_fn = (*str == '\0') ? 10 : atoi(str);

  if (!fram::exists(in_fn))
    return debug<ERROR>("file %d doesn't exist\r\n", in_fn);

  ifstream it{in_fn};

  auto out_fn = curr_proc::out_fn();
  if (out_fn == stdout) {
    while (*it != '\0') {
      serial::intr_putc(*it);
      ++it;
    }
  } else {
    ofstream os{out_fn, O_CREATE};
    while (*it != '\0') {
      os.putc(*it);
      ++it;
    }
  }

  serial::intr_putc('\r');
}
} // namespace
