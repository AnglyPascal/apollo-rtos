#include "core/test.h"
#include "fs/fs.h"
#include "utility/debug.h"

namespace
{
constexpr fn_t test_fn = 3;
constexpr size_t file_len = 16;
} // namespace

namespace TEST_SUITE(fs_sched)
{
PROC(proc0, HIGH1, 8, param)
{
  auto file = fram::open(test_fn, file_len, O_CREATE | O_WRITE);
  auto arr = (uint32_t *)file.mmap(16);

  for (uint32_t i = 0; i < 4; i++)
    *arr++ = i;

  file.store();
  file.close();
}

SYS_TEST(fs_unshared_mapping)
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
} // namespace TEST_SUITE(fs_sched)
