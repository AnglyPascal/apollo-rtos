#include "core/sched.h"
#include "core/signal.h"
#include "core/types.h"
#include "drivers/serial.h"
#include "utility/args.h"
#include "utility/iostream.h"

namespace
{
void calculator(const char *str) { printf("= %s\r\n", str); }

args_buffer_t buf{};
chan_t<1> chan;

bool listener(char c)
{
  printf("%c", c);

  if (c == CTRL('q')) {
    buf.reset();
    sched::notify(chan);
    return false;
  }

  if (c == '\r' || c == '\n') {
    printf("\r\n");

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

  clear_screen();

  while (!curr_proc::term_req()) {
    buf.reset();
    sched::wait(chan);

    calculator(buf.str);
  }

  clear_screen();
}

} // namespace

