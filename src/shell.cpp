#include "shell.h"
#include "sched.h"
#include "serial.h"
#include "types.h"
#include "waitlist.h"

namespace shell
{

void *proc(void *param)
{
  auto buf = (buffer *)param;

  auto idx = buf->find(' ');
  buf->replace(idx, '\0');
  auto [cmd, args] = buf->split(idx + 1);

  auto cmd_ptr = match_cmd(cmd);
  if (cmd_ptr == nullptr) {
    printf(">> WRONG COMMAND\r\n");
    return buf;
  }

  auto [cmd_name, priority, stk_sz, func] = *cmd_ptr;
  sched::reg_proc(cmd_name, priority, stk_sz, func, buf);
  return nullptr;
}

namespace
{
buffer *buf;

bool listener(char c)
{
  if (c == 0177) {
    printf("\b \b");
    buf->pop();
    return true;
  }

  serial::putc(c);

  if (c != '\r' && c != '\n') {
    buf->push(c);
    return true;
  }

  printf("\r\n");
  sched::reg_proc("shell", _max<priority_t>, 256, proc, buf);
  buf = new buffer{};
  return true;
}
} // namespace

void init()
{
  buf = new buffer{};
  serial::register_listener(listener);
}

} // namespace shell

