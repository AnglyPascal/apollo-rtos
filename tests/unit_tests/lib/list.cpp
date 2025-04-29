#include "utility/list.h"
#include "core/test.h"
#include "utility/debug.h"

BEGIN_SUITE(static_list)

struct _a {
  int x;
};

struct _b {
  int y;
  bool flag;
};

using chunk_t = node_t<_a>;
using mixed_chunk_t = node_t<_b>;

TEST(basic_operations)
{
  bool result = true;

  { // Basic push front/back and empty checks
    static_list_t<chunk_t> list;
    chunk_t n1({1}), n2({2}), n3({3});

    list.push_front(&n1);
    list.push_back(&n3);
    list.push_front(&n2);

    result &= (list.front()->x == 2);
    result &= (list.back()->x == 3);
    result &= !list.empty();
  }

  { // Empty list verification
    static_list_t<chunk_t> list;
    result &= list.empty();
    result &= (list.begin() == list.end());
  }

  return result;
}

TEST(iteration_functionality)
{
  bool result = true;

  { // Forward iteration with value check
    static_list_t<chunk_t> list;
    chunk_t nodes[3] = {{10}, {20}, {30}};

    list.push_back(&nodes[0]);
    list.push_back(&nodes[1]);
    list.push_back(&nodes[2]);

    int expected[] = {10, 20, 30};
    size_t i = 0;
    for (auto it = list.begin(); it != list.end(); ++it) {
      result &= expected[i++] == it->x;
    }
    result &= (i == 3);
  }

  { // Reverse iteration and mixed type
    static_list_t<mixed_chunk_t> list;
    mixed_chunk_t n1({5, true}), n2({3, false});

    list.push_front(&n2);
    list.push_front(&n1);

    // Check reverse iteration
    int expected_y[] = {3, 5};
    bool expected_flag[] = {false, true};
    int i = 0;
    for (auto it = list.rbegin(); it != list.rend(); ++it) {
      if (it->y != expected_y[i] || it->flag != expected_flag[i]) {
        result = false;
        break;
      }
      i++;
    }
    result &= (i == 2);
  }

  return result;
}

TEST(node_management)
{
  bool result = true;

  { // Detach and reattach functionality
    static_list_t<chunk_t> list;
    chunk_t a({1}), b({2});

    list.push_back(&a);
    list.push_back(&b);
    a.detach();

    // Verify remaining elements
    int expected[] = {2};
    size_t i = 0;
    for (auto it = list.begin(); it != list.end(); ++it) {
      result &= expected[i++] == it->x;
    }
    result &= i == 1;
  }

  { // Multiple detach operations
    static_list_t<chunk_t> list;
    chunk_t n1({1}), n2({2}), n3({3});

    list.push_back(&n1);
    list.push_back(&n2);
    list.push_back(&n3);
    n2.detach();

    // Verify connections
    result &= (n1.next == &n3);
    result &= (n3.prev == &n1);
  }

  {
    static_list_t<chunk_t> lst;
    chunk_t arr[4] = {{1}, {2}, {3}, {4}};

    lst.push_front(&arr[1]);
    lst.push_back(&arr[2]);
    lst.push_back(&arr[3]);
    lst.push_front(&arr[0]);

    size_t i = 0;
    while (!lst.empty()) {
      auto front = lst.front();
      front->detach();
      result &= front->x == arr[i++].x;
    }

    return result;
  }

  return result;
}

TEST(edge_cases)
{
  bool result = true;

  { // Single node list operations
    static_list_t<chunk_t> list;
    chunk_t n({42});

    list.push_back(&n);
    result &= (list.front() == list.back());
    n.detach();
    result &= list.empty();
  }

  return result;
}

END_SUITE()
