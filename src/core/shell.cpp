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

__always_inline__ inline proc_def_t *match_cmd(const char *cmd_str)
{
  for (SEC_ITER(apps, proc_def_t, proc)) {
    if (strcmp(proc->name, cmd_str) == 0)
      return proc;
  }
  return nullptr;
}

chan_t<1> chan;
args_buffer_t buf;

static const char *header = BOLD CYAN "\r\n>> " DEFAULT;

bool listener(char c)
{
  if (!buf.ready)
    return false;

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
    kprintf(header);
    return true;

  default:
    if (c < 32 && c != '\r' && c != '\n')
      return true;

    kputc(c);

    if (c >= 32 && c < 127) {
      buf.push(c);
      return true;
    }

    if (c == '\r' || c == '\n') {
      buf.ready = false;
      sched::notify(chan);
      return true;
    }

    buf.reset();
    kprintf(header);
    return true;
  }
}

SERVICE(shell, HIGH1, 256, param)
{
  serial::register_listener(listener);
  kprintf("\r\n");

  while ((volatile bool)true) {
    kprintf(header);
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

    auto param = kmem::knew<args_t>(parser.run_bg);

    auto p = parser.args;
    auto q = param->str;
    if (p != nullptr) {
      while (*p != '\0')
        *q++ = *p++;
    }
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

APP(help, MID4, 128, param)
{
  int n = 6;
  for (SEC_ITER(apps, proc_def_t, proc)) {
    kprintf("%s\t", proc->name);
    if (--n == 0) {
      kprintf("\r\n");
      n = 6;
    }
  }
  kprintf("\r\n");
}

APP(opts, MID4, 128, param)
{
  auto args = (args_t *)param;

  while (true) {
    char c = args->get_option();
    if (c == '\0')
      break;
    kputc(c);
    kputc(' ');
  }
  auto n = args->get_int();
  kprintf("n=%d, \"%s\"\r\n", n, args->s);
}

} // namespace

