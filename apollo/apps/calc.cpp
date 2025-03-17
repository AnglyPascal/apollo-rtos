#include "core/sched.h"
#include "core/shell.h"
#include "core/signal.h"
#include "core/types.h"
#include "drivers/serial.h"

namespace shell
{

namespace
{
void calculator(const char *str) { printf("= %s\r\n", str); }

args_buffer_t buf{};
chan_t<1> chan;

bool listener(char c)
{
  serial::putc(c);

  if (c == CTRL('q')) {
    buf.reset();
    sched::notify(chan);
    return false;
  }

  if (c == '\r' || c == '\n') {
    serial::putc('\r');
    serial::putc('\n');

    buf.push('\0');
    sched::notify(chan);

    return true;
  }

  buf.push(c);
  return true;
}

void calc(void *)
{
  sigterm_listener_t<__COUNTER__> sig_guard{false};
  serial::listener_guard guard{listener};

  serial::clear_screen();

  while (!curr_proc::term_req()) {
    buf.reset();
    sched::wait(chan);

    calculator(buf.str);
  }

  serial::clear_screen();
}

} // namespace

proc_def_t calc_cmd = {"calc", HIGH1, 128, calc};

} // namespace shell
