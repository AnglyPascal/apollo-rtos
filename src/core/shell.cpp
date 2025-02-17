#include "core/shell.h"

#include "core/memory.h"
#include "core/sched.h"
#include "core/types.h"
#include "drivers/serial.h"
#include "utility/debug.h"

void delay_loop(uint32_t);

__extern_C__
void trigger_reset();

// FIXME: this implementation of shell is the culprit
//
// implement it this way:
//   have a shell process, that goes into sleep immediately
//   have a uart listener, that pushes chars into a predefined buf
//   have it wake up the shell process when it gets a \r or \n
//   shell then allocates a new buffer, swaps it with the old one,
//     and passes it to the new process (if it finds one)

namespace shell
{
namespace
{
chan_t<1> chan;
args_buffer_t buf;

void shell_proc(void *)
{
  while ((volatile bool)true) {
    sched::wait(chan);

    auto str = buf.str;
    auto sz = buf.sz;

    if (sz == 0)
      continue;

    bool run_bg = false;

    while (sz >= 0 && str[sz - 1] == ' ')
      sz--;

    if (str[sz - 1] == '&') {
      run_bg = true;
      sz--;
    }

    str[sz] = '\0';

    size_t idx = 0;
    while (idx < sz && str[idx] == ' ') {
      idx++;
    }
    auto cmd = &str[idx];

    while (idx < sz && str[idx] != ' ') {
      idx++;
    }
    str[idx] = '\0';

    idx++;
    while (idx < sz && str[idx] == ' ') {
      idx++;
    }
    auto args = &str[idx];

    auto cmd_def = match_cmd(cmd);
    if (cmd_def == nullptr) {
      debug<ERROR>("wrong command: \"%s\"\r\n", cmd);
    } else {
      auto param = kmem::knew<args_t>();
      param->run_bg = run_bg;

      size_t i = 0;
      while (*args != '\0') {
        param->str[i++] = *args++;
      }
      param->str[i++] = '\0';

      sched::reg_proc(cmd_def, param);
    }

    printf("\r\n");
    buf.reset();
  }
}

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

proc_def_t proc_def = {"shell", HIGH1, 256, shell_proc};

} // namespace

void init()
{
  buf.reset();
  serial::register_listener(listener);
  sched::reg_proc(&proc_def, nullptr);
}

} // namespace shell

