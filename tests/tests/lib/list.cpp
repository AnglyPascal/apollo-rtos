#include "core/test.h"
#include "utility/debug.h"
#include "utility/list.h"

BEGIN_SUITE(static_list)

struct _a {
  int x;
};
using chunk_t = node_t<_a>;

TEST(basic)
{
  static_list_t<chunk_t> lst;
  chunk_t arr[4] = {{1}, {2}, {3}, {4}};

  lst.push_front(&arr[1]);
  lst.push_back(&arr[2]);
  lst.push_back(&arr[3]);
  lst.push_front(&arr[0]);

  size_t i = 0;
  bool result = true;
  while (!lst.empty()) {
    auto front = lst.front();
    front->detach();
    result &= front->x == arr[i++].x;
  }

  return result;
}

TEST(iterator)
{
  static_list_t<chunk_t> lst;
  chunk_t arr[4] = {{1}, {2}, {3}, {4}};

  lst.push_front(&arr[1]);
  lst.push_back(&arr[2]);
  lst.push_back(&arr[3]);
  lst.push_front(&arr[0]);

  size_t i = 0;
  bool result = true;

  for (auto it = lst.begin(); it != lst.end(); ++it)
    result &= it->x == arr[i++].x;
  result &= i == 4;

  for (auto it = lst.rbegin(); it != lst.rend(); ++it)
    result &= it->x == arr[--i].x;
  result &= i == 0;

  return result;
}

END_SUITE()
