#include "core/test.h"
#include "fs/fs.h"
#include "utility/debug.h"
#include "utility/iostream.h"

namespace
{
constexpr fn_t test_fn = 3;
}

namespace sched
{
void set_out_fn(pid_t, fn_t);
}

BEGIN_SUITE(ofstream)

TEST(basic_write)
{
  bool result = true;

  {
    ofstream os{test_fn, O_CREATE | O_CHAR_FILE};
    os.printf("hello, world, %d!", 2025);
  }

  {
    const char *str = "hello, world, 2025!";
    auto s = str;

    ifstream it{test_fn};
    while (*it != '\0') {
      result &= *s++ == *it;
      ++it;
    }
  }

  fram::remove(test_fn);
  return result;
}

PROC(proc0, MID3, 0, param)
{
  auto [str, substr] = *static_cast<pair<const char *, const char *> *>(param);
  printf(str, substr);
}

SYS_TEST(write_redirection)
{
  auto str = "this is a %s string to write to the file\n";
  auto substr = "formatted";

  auto p0 = REG_PROC(proc0, pair<const char *, const char *>(str, substr));
  sched::set_out_fn(p0, test_fn);

  sched::wait(p0);

  auto total = "this is a formatted string to write to the file\n";
  bool result = true;
  {
    ifstream it{test_fn};
    auto t = total;
    while (*it != '\0') {
      result &= *t++ == *it;
      ++it;
    }
  }

  fram::remove(test_fn);
  return result;
}

END_SUITE()
