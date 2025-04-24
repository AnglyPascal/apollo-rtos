#include "core/memory.h"
#include "core/sched.h"
#include "core/types.h"
#include "drivers/serial.h"
#include "utility/args.h"
#include "utility/debug.h"
#include "utility/iostream.h"

namespace sched
{
void set_out_fn(pid_t pid, fn_t fn);
}

namespace
{
SEC_ADDR(apps);

__always_inline__ inline proc_def_t *match_cmd(string cmd_str)
{
  for (SEC_ITER(apps, proc_def_t, proc)) {
    if (proc->name == cmd_str)
      return proc;
  }
  return nullptr;
}

chan_t<1> chan;
args_buffer_t buf;
static_assert(sizeof(buf) == 0x44);

bool listener(char c)
{
  switch (c) {
  case DEL:
  case BS:
    kprintf("\b \b");
    buf.pop();
    return true;

  case CTRL('d'):
    clear_screen();
    trigger_reset();
    return true;

  case CTRL('l'):
    clear_screen();
    return true;

  case CTRL('c'):
    buf.reset();
    kprintf("^C\r\n");
    return true;

  default:
    kprintf("%c", c);

    if (c >= 32 && c < 127) {
      buf.push(c);
      return true;
    }

    if (c == '\r' || c == '\n') {
      sched::notify(chan);
      return true;
    }

    kprintf("\'%d\'\r\n", c);
    buf.reset();
    return true;
  }
}

SERVICE(shell, HIGH1, 256, param)
{
  serial::register_listener(listener);
  kprintf("\r\n");

  while ((volatile bool)true) {
    kprintf(">> ");
    buf.reset();
    sched::wait(chan);

    if (buf.sz == 0)
      continue;

    kprintf("\r\n");

    buf.str[buf.sz] = '\0';
    parser_t parser{buf.str};

    auto cmd_def = match_cmd(parser.cmd);
    if (cmd_def == nullptr) {
      kprintf("wrong command: " RED "%s" DEFAULT "\r\n", parser.cmd);
      continue;
    }

    auto param = kmem::knew<args_t>();
    param->run_bg = parser.run_bg;

    auto p = parser.args;
    auto q = param->str;
    while (*p != '\0')
      *q++ = *p++;
    *q = '\0';

    auto pid = sched::reg_proc(cmd_def, (void *)param);

    if (parser.fn != null_fn)
      sched::set_out_fn(pid, parser.fn);

    if (parser.run_bg) {
      kprintf("started [" BLUE "%d" DEFAULT "] output to ", pid);
      parser.fn != null_fn ? kprintf(BLUE "%d\r\n" DEFAULT, parser.fn)
                           : kprintf(MAGENTA "stdout\r\n" DEFAULT);
    } else {
      sched::wait(pid);
    }
  }
}

} // namespace

