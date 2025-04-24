#include "core/memory.h"
#include "core/test.h"
#include "utility/args.h"
#include "utility/debug.h"

BEGIN_SUITE(args_parser)

char buffer[64];

struct expected_t {
  const char *str;
  const char *cmd;
  const char *args;
  fn_t fn;
  bool bg;

  bool test()
  {
    strcpy(buffer, str);
    parser_t parser{buffer};

    bool result = true;

    result &=
        cmd == nullptr ? parser.cmd == nullptr : strcmp(cmd, parser.cmd) == 0;
    result &= args == nullptr ? parser.args == nullptr
                              : strcmp(args, parser.args) == 0;
    result &= fn == parser.fn;
    result &= bg == parser.run_bg;

    if (!result)
      kprintf("cmd: %s, args: %s, fn: %d, bg: %d\r\n", parser.cmd, parser.args,
              parser.fn, parser.run_bg);

    return result;
  }
};

static expected_t exps[] = {
    {" cmd args > 100 &  ", "cmd", "args", 100, true},
    {"cmd", "cmd", nullptr, null_fn, false},
    {"cmd args here", "cmd", "args here", null_fn, false},
    {"cmd >123", "cmd", nullptr, 123, false},
    {"cmd &", "cmd", nullptr, null_fn, true},
    {"cmd some_args >42", "cmd", "some_args", 42, false},
    {"cmd some_args&", "cmd", "some_args", null_fn, true},
    {"cmd >7 &", "cmd", nullptr, 7, true},
    {" cmd args >100 &  ", "cmd", "args", 100, true},
    {"   cmd   args   >   250   &   ", "cmd", "args", 250, true},
    {"cmd >abc &", "cmd", nullptr, null_fn, false},
    {"cmd >256 &", "cmd", nullptr, null_fn, false},
    {"cmd arg>ment >123 &", "cmd", "arg", null_fn, false},
    {"cmd arg&ment &", "cmd", "arg", null_fn, true},
    {"", nullptr, nullptr, null_fn, false},
    {"   ", nullptr, nullptr, null_fn, false},
    {">&", nullptr, nullptr, null_fn, false},
};

TEST(all_cases)
{
  bool result = true;
  constexpr auto N = sizeof(exps) / sizeof(exps[0]);
  for (size_t i = 0; i < N; i++) {
    result &= exps[i].test();
  }
  return result;
}

END_SUITE()
