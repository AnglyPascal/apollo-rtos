#include "utility/circular_buffer.h"
#include "core/test.h"
#include "utility/debug.h"

BEGIN_SUITE(circular_buffer)

TEST(basic_operations)
{
  bool result = true;

  { // Basic write/read and size checks
    circular_buffer<int, 4> buf;
    result &= buf.empty();

    buf.enqueue(1);
    buf.enqueue(2);
    result &= (buf.size() == 2);
    result &= (buf.dequeue() == 1);
    result &= (buf.dequeue() == 2);
    result &= buf.empty();
  }

  { // Mixed data types
    struct _a {
      int x;
      int y;
      bool z;
    };
    circular_buffer<_a, 3> buf;

    buf.enqueue(1, 2, true);
    buf.enqueue(3, 4, false);
    _a item = buf.dequeue();
    result &= (item.x == 1 && item.y == 2 && item.z);
    result &= (buf.size() == 1);
  }

  return result;
}

TEST(capacity_and_overwrite)
{
  bool result = true;

  { // Exact capacity fill
    circular_buffer<int, 3> buf;
    int expected[] = {1, 2, 3};
    for (int i = 1; i <= 3; i++)
      buf.enqueue(i);

    for (int i = 0; i < 3; i++)
      result &= expected[i] == buf.dequeue();
  }

  { // Overwrite behavior
    circular_buffer<int, 3> buf;
    int expected[] = {3, 4, 5};
    for (int i = 1; i <= 5; i++)
      buf.enqueue(i);

    for (int i = 0; i < 3; i++)
      result &= expected[i] == buf.dequeue();
  }

  return result;
}

TEST(wrap_around_and_iteration)
{
  bool result = true;

  { // Wrap-around scenario
    circular_buffer<int, 4> buf;
    int pattern[] = {1, 2, 3, 4, 5, 6};
    int expected[] = {3, 4, 5, 6};

    for (int i = 0; i < 6; i++)
      buf.enqueue(pattern[i]);

    for (int i = 0; i < 4; i++)
      result &= expected[i] == buf.dequeue();
  }

  { // Iterator functionality with struct
    struct _a {
      int x;
      bool y;
    };
    circular_buffer<_a, 3> buf;
    _a expected[] = {{1, true}, {2, false}, {3, true}};

    buf.enqueue(expected[0]);
    buf.enqueue(expected[1]);
    buf.enqueue(expected[2]);

    int i = 0;
    for (auto it = buf.begin(); it != buf.end(); ++it) {
      auto [x, y] = *it;
      auto [ex, ey] = expected[i++];
      result &= (x == ex) && (y == ey);
    }
    result &= (i == 3);
  }

  return result;
}

TEST(edge_cases)
{
  bool result = true;

  { // Single element buffer
    circular_buffer<int, 1> buf;
    buf.enqueue(5);
    result &= (buf.dequeue() == 5);
    buf.enqueue(6);
    result &= (buf.dequeue() == 6);
    result &= buf.empty();
  }

  { // Peek and front/back with struct
    struct _a {
      int x;
      int y;
      bool z;
    };
    circular_buffer<_a, 2> buf;

    buf.enqueue(1, 2, true);
    result &= (buf.peek().x == 1);
    buf.enqueue(3, 4, false);
    result &= (buf.front().y == 2);
    result &= (buf.back().z == false);
  }

  { // Clear behavior
    circular_buffer<int, 3> buf;
    buf.enqueue(1);
    buf.enqueue(2);
    while (!buf.empty())
      buf.dequeue();
    result &= (buf.size() == 0);
    result &= (buf.begin() == buf.end());
  }

  return result;
}

END_SUITE()
