#include "shell.h"
#include "char_buffer.h"
#include "sched.h"
#include "serial.h"
#include "types.h"

namespace shell
{

void proc(void *param)
{
  auto buf = (char_buffer<args_len> *)param;

  auto idx = buf->find(' ');
  buf->replace(idx, '\0');
  auto [cmd, args] = buf->split(idx + 1);

  auto cmd_ptr = match_cmd(cmd);

  if (cmd_ptr == nullptr) {
    printf("\r\n!!! WRONG COMMAND\r\n");
    /* printf("\r\n!!! wrong command, \"%x\"\r\n", (uint32_t)buf); */
    delete buf;
    return;
  }

  auto [cmd_name, priority, stk_sz, func] = *cmd_ptr;
  printf("\r\n");
  sched::reg_proc(cmd_name, priority, stk_sz, func, buf);
}

namespace
{
char_buffer<args_len> *buf;
}

void init()
{
  buf = new char_buffer<args_len>{};
  printf("%x, %d\r\n", buf, buf->sz);
}

void shell_listener(char c)
{
  if (c != '\r' && c != '\n')
    return buf->push(c);

  printf("\r\n");
  sched::reg_proc("shell", 8, 256, proc, buf);
  buf = new char_buffer<args_len>{};
  printf("%x, %d\r\n", buf, buf->sz);
}

} // namespace shell

namespace serial
{

void listener(char c)
{
  shell::shell_listener(c);
}

} // namespace serial
