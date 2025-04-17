#include "core/test.h"
#include "fs/fs.h"
#include "utility/debug.h"

namespace
{
constexpr fn_t test_fn = 3;
}

namespace TEST_SUITE(char_file)
{

TEST(char_file_creation)
{
  auto file = fram::open(test_fn, 256, O_CREATE | O_CHAR_FILE | O_APPEND);

  // constexpr auto str = "this is a string that needs to be stored in the file";
  // file.write(str, strlen(str));

  // auto it = file.read(strlen(str));
  // while (*it != '\0')
  //   it++;

  fram::remove(test_fn);
  return true;
}

} // namespace TEST_SUITE(char_file)
