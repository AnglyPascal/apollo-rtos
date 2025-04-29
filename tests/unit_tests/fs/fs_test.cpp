#include "core/sched.h"
#include "core/test.h"
#include "fs/fs.h"
#include "utility/debug.h"

namespace
{
constexpr fn_t test_fn = 3;
}

BEGIN_SUITE(fs_basic)

const char *const file_content = "this is a file";
constexpr size_t file_len = 16;

TEST(open_store_close)
{
  const char *t0;

  {
    auto file = fram::open(test_fn, file_len, O_CREATE | O_WRITE | O_SHARED);
    auto t = (char *)file.mmap(file_len);
    t0 = t;

    auto s = file_content;
    while (*s != '\0')
      *t++ = *s++;
    *t = '\0';

    file.store();
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
  }

  bool equal = true;

  {
    auto file = fram::open(test_fn, O_READ | O_SHARED);
    auto t = (uint32_t *)file.mmap(file_len);

    for (size_t i = 0; i < file_len / sizeof(*t); i++)
      equal &= t[i] == val;
  }

  fram::remove(test_fn);

  return equal;
}

TEST(mmap)
{
  auto file = fram::open(test_fn, file_len, O_CREATE | O_WRITE | O_SHARED);

  auto t = file.mmap(file_len);
  auto s = file.mmap(file_len);

  file.close();
  fram::remove(test_fn);
  return t == s;
}

END_SUITE()

namespace
{
constexpr size_t file_len = 16;
} // namespace

BEGIN_SUITE(fs_mapping)

PROC(proc0, HIGH1, 128, param)
{
  auto file = fram::open(test_fn, file_len, O_CREATE | O_WRITE);
  auto arr = (uint32_t *)file.mmap(16);

  for (uint32_t i = 0; i < 4; i++)
    *arr++ = i;

  file.store();
}

SYS_TEST(unshared_mapping)
{
  bool result = true;

  auto p0 = REG_PROC(proc0, nullptr);
  sched::wait(p0);

  auto file = fram::open(test_fn, O_READ);
  auto arr = (uint32_t *)file.mmap(file_len);

  for (uint32_t i = 0; i < file_len / sizeof(*arr); i++)
    result &= (*arr++ == i);

  file.close();
  fram::remove(test_fn);

  return result;
}

END_SUITE()

BEGIN_SUITE(fs_contention)

constexpr size_t N = 128;
constexpr size_t n = 4;

PROC(proc0, MID4, 128, param)
{
  auto str = *(const char **)param;
  auto file = fram::open(test_fn, O_APPEND);
  for (size_t i = 0; i < n; i++)     
    file.append(str, N);
}

char s0[N + 1], s1[N + 1], s2[N + 1], s3[N + 1];

SYS_TEST(write_contention)
{
  constexpr auto len = N * n * 4 + 8;

  auto file = fram::open(test_fn, len, O_CREATE | O_CHAR_FILE | O_APPEND);
  file.append("S", 1);

  for (size_t i = 0; i < N; i++) {
    s0[i] = '0';
    s1[i] = '1';
    s2[i] = '2';
    s3[i] = '3';
  }
  s0[N] = '\0';
  s1[N] = '\0';
  s2[N] = '\0';
  s3[N] = '\0';

  auto p0 = REG_PROC(proc0, (const char *)s0);
  auto p1 = REG_PROC(proc0, (const char *)s1);
  auto p2 = REG_PROC(proc0, (const char *)s2);
  auto p3 = REG_PROC(proc0, (const char *)s3);
  sched::wait(p0, p1, p2, p3);

  file.append("\0", 1);
  auto str = (const char *const)file.mmap(len);

  constexpr auto exp = (N * n * 4) + 1;
  auto res = exp == strlen(str);

  file.close();
  fram::remove(test_fn);

  return res;
}

END_SUITE()
