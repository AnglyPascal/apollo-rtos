#include "core/memory.h"
#include "core/sched.h"
#include "core/types.h"
#include "drivers/serial.h"
#include "utility/args.h"
#include "utility/debug.h"

namespace shell
{
namespace
{
__extern_C__ proc_def_t __apps_load[], __apps_start[], __apps_end[];

proc_def_t *match_cmd(string cmd_str)
{
  for (auto proc = __apps_start; proc < __apps_end; proc++) {
    if (proc->name == cmd_str)
      return proc;
  }
  return nullptr;
}

chan_t<1> chan;
args_buffer_t buf;
static_assert(sizeof(buf) == 0x44);

bool listener(char c);

STARTUP(shell, HIGH1, 256, param)
{
  serial::register_listener(listener);

  while ((volatile bool)true) {
    buf.reset();
    sched::wait(chan);

    auto str = buf.str;
    auto sz = buf.sz;

    if (sz == 0)
      continue;

    bool run_bg = false;

    // trim trailing spaces
    while (sz >= 0 && str[sz - 1] == ' ')
      sz--;

    // run in background?
    if (str[sz - 1] == '&') {
      run_bg = true;
      sz--;
    }
    str[sz] = '\0';

    // trim leading spaces
    size_t idx = 0;
    while (idx < sz && str[idx] == ' ')
      idx++;

    // find end of command
    auto cmd = &str[idx];
    while (idx < sz && str[idx] != ' ')
      idx++;
    str[idx] = '\0';

    auto cmd_def = match_cmd(cmd);
    if (cmd_def == nullptr) {
      debug<ERROR>("\r\nwrong command: \"%s\"\r\n", cmd);
      continue;
    }

    auto param = kmem::knew<args_t>();
    param->run_bg = run_bg;

    // trim leading spaces from args
    idx++;
    while (idx < sz && str[idx] == ' ')
      idx++;

    size_t i = 0;
    while (idx < sz)
      param->str[i++] = str[idx++];
    param->str[i++] = '\0';

    sched::reg_proc(cmd_def, param);

    printf("\r\n");
  }
}

bool listener(char c)
{
  if (c == DEL || c == BS) {
    printf("\b \b");
    buf.pop();
    return true;
  }

  if (c == CTRL('d')) {
    clear_screen();
    trigger_reset();
    return true;
  }

  if (c == CTRL('l')) {
    clear_screen();
    return true;
  }

  if (c == CTRL('c')) {
    buf.reset();
    printf("^C\r\n");
    return true;
  }

  serial::putc(c);

  if (c >= 32 && c < 127) {
    buf.push(c);
    return true;
  }

  if (c == '\r' || c == '\n') {
    sched::notify(chan);
    return true;
  }

  printf("\'%d\'\r\n", c);
  buf.reset();

  return true;
}

} // namespace
} // namespace shell

