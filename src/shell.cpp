#include "shell.h"
#include "debug.h"
#include "memory.h"
#include "sched.h"
#include "serial.h"
#include "types.h"
#include "waitlist.h"

void delay_loop(uint32_t);

__extern_C__
void trigger_reset();

namespace shell
{

void proc(void *param)
{
  auto &buf = *(args_buffer_t *)param;

  size_t idx = 0;
  while (idx < buf.sz && buf[idx] != ' ') {
    idx++;
  }

  buf[idx] = '\0';

  idx++;
  while (idx < buf.sz && buf[idx] == ' ') {
    idx++;
  }
  buf.args = &buf[idx];

  if (buf[buf.sz - 1] == '&') {
    buf.run_bg = true;

    buf.pop();
    buf.push('\0');
  }

  auto cmd = buf.str;
  auto cmd_def = match_cmd(cmd);
  if (cmd_def == nullptr) {
    debug<ERROR>("wrong command: \"%s\"\r\n", cmd);
    return;
  }

  cmd_def->param = param;
  sched::reg_proc(cmd_def);
  cmd_def->param = nullptr;

  sched::transfer_param(param);
}

namespace
{
args_buffer_t *buf;

bool listener(char c)
{
  if (c == 0177) {
    printf("\b \b");
    buf->pop();
    return true;
  }

  if (c == CTRL('d')) {
    serial::clear_screen();
    trigger_reset();
    return true;
  }

  serial::putc(c);

  if (c != '\r' && c != '\n') {
    buf->push(c);
    return true;
  }

  kprintf("\r\n");
  sched::reg_proc("shell", _max<priority_t>, 256, proc, buf);

  buf = (args_buffer_t *)kmem::kmalloc(sizeof(args_buffer_t));
  buf->reset();

  return true;
}
} // namespace

void init()
{
  buf = (args_buffer_t *)kmem::kmalloc(sizeof(args_buffer_t));
  buf->reset();

  serial::register_listener(listener);
}

} // namespace shell

