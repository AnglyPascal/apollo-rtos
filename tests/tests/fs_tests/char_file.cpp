#include "core/test.h"
#include "fs/fs.h"
#include "utility/debug.h"

namespace
{
constexpr fn_t test_fn = 3;
}

namespace TEST_SUITE(char_file)
{
constexpr auto str1 = "this is a string that needs to be stored in the file; ";
constexpr auto str2 = "this is the second string that goes in the file.";
constexpr auto str3 = "THIS IS A STRING";
char tmp[strlen(str1) + strlen(str2) + 1] = {0};

TEST(char_file_creation)
{
  bool result = true;

  {
    auto file = fram::open(test_fn, 256, O_CREATE | O_CHAR_FILE | O_APPEND);
    file.append(str1, strlen(str1));
  }

  {
    auto file = fram::open(test_fn, 256, O_CHAR_FILE | O_APPEND);
    file.append(str2, strlen(str2) + 1);
  }

  {
    auto file = fram::open(test_fn, 256, O_CHAR_FILE | O_APPEND);
    {
      auto it = file.read(strlen(str1) + strlen(str2));

      size_t idx = 0;
      while (*it != '\0') {
        tmp[idx++] = *it;
        ++it;
      }
      result &= idx == strlen(str1) + strlen(str2);
      tmp[idx] = '\0';
    }
  }

  {
    size_t idx = 0;
    while (idx < strlen(str1)) {
      result &= tmp[idx] == str1[idx];
      idx++;
    }
    while (idx < strlen(str2)) {
      result &= tmp[idx] == str2[idx];
      idx++;
    }
  }

  {
    auto file = fram::open(test_fn, 256, O_CHAR_FILE | O_APPEND);
    file.write(str3, strlen(str3), 0);
  }

  {
    auto file = fram::open(test_fn, 256, O_CHAR_FILE | O_APPEND);
    {
      auto it = file.read(strlen(str3));

      size_t idx = 0;
      while (*it != '\0') {
        tmp[idx++] = *it;
        ++it;
      }
      result &= idx == strlen(str3);
    }
  }

  {
    size_t idx = 0;
    while (idx < strlen(str3)) {
      result &= tmp[idx] == str3[idx];
      idx++;
    }
    while (idx < strlen(str1)) {
      result &= tmp[idx] == str1[idx];
      idx++;
    }
    while (idx < strlen(str2)) {
      result &= tmp[idx] == str2[idx];
      idx++;
    }
  }

  fram::remove(test_fn);
  return result;
}

} // namespace TEST_SUITE(char_file)
