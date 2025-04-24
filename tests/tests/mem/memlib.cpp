#include "core/memory.h"
#include "core/test.h"

BEGIN_SUITE(memlib)

struct A {
  int x;
  bool b;
  char c;

  bool operator==(const A &other) const
  {
    return x == other.x && b == other.b && c == other.c;
  }
};

TEST(memcpy_test)
{
  bool result = true;

  constexpr size_t N = 11;
  int arr[N];
  for (size_t i = 0; i < N; i++)
    arr[i] = i;
  int brr[N];
  memcpy(brr, arr, sizeof(arr));
  result &= memcmp(arr, brr, sizeof(arr)) == 0;

  char src[10] = "abcdefghi";
  char dst[10];
  memcpy(dst, src, 5);
  result &= memcmp(src, dst, 5) == 0;
  result &= dst[5] != src[5];

  constexpr size_t N2 = 5;
  A s1[N2] = {
      {41, true, 'A'}, {42, true, 'B'},  {43, false, 'B'},
      {44, true, 'C'}, {45, false, 'D'},
  };
  A s2[N2];
  memcpy(&s1, &s2, sizeof(s1));
  for (size_t i = 0; i < N2; i++)
    result &= s1[i] == s2[i];

  return result;
}

TEST(memset_test)
{
  bool result = true;

  char buf[10];
  memset(buf, 0xAA, sizeof(buf));
  for (auto &c : buf)
    result &= (c == 0xAA);

  int arr[5];
  memset(arr, 0xFF, sizeof(arr));
  const uint8_t *p = reinterpret_cast<const uint8_t *>(arr);
  for (size_t i = 0; i < sizeof(arr); i++)
    result &= (p[i] == 0xFF);

  A s;
  memset(&s, 0, sizeof(A));
  result &= (s.x == 0) && (s.b == false) && (s.c == '\0');

  return result;
}

TEST(memcmp_test)
{
  bool result = true;

  const int a[3] = {1, 2, 3};
  const int b[3] = {1, 2, 3};
  result &= memcmp(a, b, sizeof(a)) == 0;

  const int c[3] = {1, 1, 3};
  const int d[3] = {1, 2, 0};
  result &= memcmp(c, d, sizeof(c)) < 0;

  const char str1[] = "hello";
  const char str2[] = "hella";
  result &= memcmp(str1, str2, 5) > 0;

  result &= memcmp("apple", "applesauce", 5) == 0;

  return result;
}

TEST(strcpy_test)
{
  bool result = true;

  char dest[20];
  strcpy(dest, "Hello World");
  result &= strcmp(dest, "Hello World") == 0;

  strcpy(dest, "");
  result &= strcmp(dest, "") == 0;

  strcpy(dest, "abcdef");
  strcpy(dest + 3, "abcdef");
  result &= strcmp(dest, "abcabcdef") == 0;

  return result;
}

TEST(strcmp_test)
{
  bool result = true;

  result &= strcmp("test", "test") == 0;

  result &= strcmp("apple", "apples") < 0;
  result &= strcmp("zebra", "apple") > 0;

  result &= strcmp("apple_pinapple", "apple") > 0;
  result &= strcmp("apple", "apple_pinapple") < 0;

  result &= strcmp("HELLO", "hello") < 0;

  return result;
}

END_SUITE()
