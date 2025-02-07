#include "memory.h"
#include "serial.h"
#include "shell.h"
#include "types.h"
#include "waitlist.h"

namespace shell
{

namespace
{

volatile bool exit = false;

// FIXME: will run even when the process is not running, kind of a bummer
void calculator(const char *str)
{
  printf("= %s\r\n", str);
}

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

  if (c == CTRL('q')) {
    exit = true;
    return true;
  }

  buf.push(c);
  return false;
}

void calc(void *param)
{
  serial::listener_guard guard{listener};
  serial::clear_screen();

  while (!exit) {
    sched::sleep(100);
  }

  serial::clear_screen();
  exit = false;
}

} // namespace

proc_def_t calc_cmd = {"calc", 4, 128, calc, nullptr};

} // namespace shell
