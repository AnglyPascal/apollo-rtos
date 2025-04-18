#include "core/test.h"
#include "fs/fs.h"
#include "utility/debug.h"

namespace
{
constexpr fn_t test_fn = 3;
}

namespace TEST_SUITE(ofstream_test)
{
TEST(ofstream_concept)
{
  bool result = true;

  {
    fram::open(test_fn, 256, O_WRITE | O_CREATE | O_CHAR_FILE);
  }

  {
    ofstream os{test_fn};
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
} // namespace TEST_SUITE(ofstream_test)
