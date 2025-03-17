#include "core/sched.h"
#include "core/shell.h"
#include "core/signal.h"
#include "core/types.h"
#include "drivers/serial.h"

namespace
{
void calculator(const char *str) { printf("= %s\r\n", str); }

shell::args_buffer_t buf{};
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

APP(calc, HIGH1, 128, param)
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

