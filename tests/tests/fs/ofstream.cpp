#include "core/test.h"
#include "fs/fs.h"
#include "utility/debug.h"

namespace
{
constexpr fn_t test_fn = 3;
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

END_SUITE()
