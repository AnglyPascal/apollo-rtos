#include "core/test.h"
#include "fs/fs.h"
#include "utility/debug.h"

namespace
{
constexpr fn_t test_fn = 3;
}

BEGIN_SUITE(fs_pre_sched)

const char *const file_content = "this is a file";

TEST(open_store_close)
{
  const char *t0;
  constexpr size_t file_len = 16;

  {
    auto file = fram::open(test_fn, file_len, O_CREATE | O_WRITE | O_SHARED);
    auto t = (char *)file.mmap(file_len);
    t0 = t;

    auto s = file_content;
    while (*s != '\0')
      *t++ = *s++;
    *t = '\0';

    file.store();
    file.close();
  }

  bool res = true;

  {
    auto file = fram::open(test_fn, O_READ | O_SHARED);
    auto t = (char *)file.mmap(file_len);

    res &= t0 == t;

    auto s = file_content;
    while (*t != '\0' && *t++ == *s++)
      ;

    res &= *t == '\0' && *s == '\0';

    file.close();
  }

  fram::remove(test_fn);

  return res;
}

TEST(mutli_blk_file)
{
  constexpr uint32_t val = 0xABCD0123;
  constexpr size_t file_len = 256;

  {
    auto file = fram::open(test_fn, file_len, O_WRITE | O_CREATE | O_SHARED);
    auto t = (uint32_t *)file.mmap(file_len);

    for (size_t i = 0; i < file_len / sizeof(*t); i++)
      *t++ = val;

    file.store();
    file.close();
  }

  bool equal = true;

  {
    auto file = fram::open(test_fn, O_READ | O_SHARED);
    auto t = (uint32_t *)file.mmap(file_len);

    for (size_t i = 0; i < file_len / sizeof(*t); i++)
      equal &= t[i] == val;

    file.close();
  }

  fram::remove(test_fn);

  return equal;
}

END_SUITE()
