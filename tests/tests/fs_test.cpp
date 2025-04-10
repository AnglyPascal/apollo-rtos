#include "core/test.h"
#include "fs/fs.h"
#include "utility/debug.h"

namespace
{
constexpr fn_t test_fn = 3;
const char *const file_content = "this is a file";

TEST(fs_open_store_close)
{
  bool res = true;
  const char *t0;

  {
    auto file = fram::open(test_fn, 16, O_CREATE | O_WRITE | O_SHARED);
    auto t = (char *)file.mmap(16);
    t0 = t;

    auto s = file_content;
    while (*s != '\0')
      *t++ = *s++;
    *t = '\0';

    file.store();
    file.close();
  }

  {
    auto file = fram::open(test_fn, 16, O_READ | O_SHARED);
    auto t = (char *)file.mmap(16);

    res = res && t0 == t;

    auto s = file_content;
    while (*t != '\0' && *t++ == *s++)
      ;

    res = res && *t == '\0' && *s == '\0';

    file.close();
  }

  fram::remove(test_fn);

  return res;
}

TEST(fs_mutli_blk_file)
{
  {
    auto file = fram::open(test_fn, 256, O_WRITE | O_CREATE | O_SHARED);
    auto t = (uint32_t *)file.mmap(256);

    constexpr uint32_t val = 0xABCD0123;

    for (size_t i = 0; i < 256 / sizeof(*t); i++)
      *t++ = val;

    file.store();
    file.close();
  }

  bool equal = true;

  {
    auto file = fram::open(test_fn, 256, O_READ | O_SHARED);
    auto t = (uint32_t *)file.mmap(256);

    constexpr uint32_t val = 0xABCD0123;

    for (size_t i = 0; i < 256 / sizeof(*t); i++)
      equal &= t[i] == val;

    file.store();
    file.close();
  }

  fram::remove(test_fn);

  return equal;
}

} // namespace

