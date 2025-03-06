#include "core/sched.h"
#include "core/shell.h"
#include "core/signal.h"
#include "core/types.h"
#include "drivers/serial.h"

namespace shell
{

namespace
{
// FIXME: will run even when the process is not running, kind of a bummer
void calculator(const char *str) { printf("= %s\r\n", str); }

args_buffer_t buf{};

bool listener(char c)
{
  serial::putc(c);

  if (c == '\r' || c == '\n') {
    serial::putc('\r');
    serial::putc('\n');

    buf.push('\0');
    calculator(buf.str);
    buf.reset();
    return true;
  }

  buf.push(c);
  return true;
}

void calc(void *)
{
  serial::listener_guard guard{listener};
  sigterm_listener_t<__COUNTER__> sig_guard{false};

  serial::clear_screen();

  while (!curr_proc::term_req()) {
    sched::sleep(100);
  }

  serial::clear_screen();
}

} // namespace

proc_def_t calc_cmd = {"calc", HIGH1, 128, calc};

} // namespace shell
