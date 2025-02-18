#include "core/shell.h"

#include "core/memory.h"
#include "core/sched.h"
#include "core/types.h"
#include "drivers/serial.h"
#include "utility/debug.h"

namespace shell
{
namespace
{
chan_t<1> chan;
args_buffer_t buf;

bool listener(char c);

void shell_proc(void *)
{
  buf.reset();
  serial::register_listener(listener);

  while ((volatile bool)true) {
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
      buf.reset();
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
    buf.reset();
  }
}

__extern_C__
void trigger_reset();

bool listener(char c)
{
  if (c == 0177) {
    printf("\b \b");
    buf.pop();
    return true;
  }

  if (c == CTRL('d')) {
    serial::clear_screen();
    trigger_reset();
    return true;
  }

  serial::putc(c);

  if (c != '\r' && c != '\n') {
    buf.push(c);
    return true;
  }

  sched::notify(chan);

  return true;
}

proc_def_t shell_def = {"shell", HIGH1, 256, shell_proc};
} // namespace

void init() { sched::reg_proc(&shell_def, nullptr); }

} // namespace shell

