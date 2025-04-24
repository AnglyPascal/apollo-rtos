#include "utility/stack.h"
#include "core/test.h"

BEGIN_SUITE(stack)

TEST(basic_integer_operations)
{
  bool result = true;

  { // Small stack with basic push/pop
    stack<int, 3> s;
    auto t1 = s.push(1);
    auto t2 = s.push(2);
    auto t3 = s.push(3);

    result &= *t3 == 3 && (s.pop() == 3);
    result &= *t2 == 2 && (s.pop() == 2);
    result &= *t1 == 1 && (s.pop() == 1);
    result &= s.empty();
  }

  { // Size tracking verification
    stack<int, 5> s;
    result &= (s.size() == 0);
    s.push(10);
    s.push(20);
    result &= (s.size() == 2);
    s.pop();
    result &= (s.size() == 1);
  }

  return result;
}

struct A {
  int x;
  bool y;
  char z;
};

TEST(struct_operations)
{
  bool result = true;

  { // Complex struct with initialization
    stack<A, 2> s;
    s.push(5, true, 'A');
    s.push(10, false, 'B');

    auto top = s.top();
    result &= (top.x == 10 && !top.y && top.z == 'B');
    s.pop();
    result &= (s.pop().x == 5);
  }

  { // Mixed struct operations
    stack<A, 3> s;
    s.push(1, true, 'X');
    s.push(2, false, 'Y');
    s.pop();
    s.push(3, true, 'Z');

    result &= (s.size() == 2);
    result &= (s.pop().z == 'Z');
    result &= (s.pop().x == 1);
  }

  return result;
}

TEST(edge_cases)
{
  bool result = true;

  { // Single-element stack
    stack<char, 1> s;
    s.push('A');
    result &= s.push('B') == nullptr;
    result &= (s.size() == 1);
    result &= (s.pop() == 'A');
  }

  { // Empty stack handling
    stack<float, 2> s;
    result &= s.empty();
    result &= (s.size() == 0);
  }

  { // Fill and empty cycle
    stack<int, 4> s;
    for (int i = 0; i < 4; i++)
      s.push(i);
    for (int i = 3; i >= 0; i--)
      result &= (s.pop() == i);
    for (int i = 0; i < 4; i++)
      s.push(i + 10);
    for (int i = 13; i >= 10; i--)
      result &= (s.pop() == i);
  }

  return result;
}

TEST(order_preservation)
{
  bool result = true;

  { // Verify LIFO behavior with array
    stack<int, 5> s;
    int inputs[] = {1, 2, 3, 4, 5};
    int expected[] = {5, 4, 3, 2, 1};

    for (auto v : inputs)
      s.push(v);
    for (auto e : expected)
      result &= (s.pop() == e);
  }

  { // Mixed push/pop sequence
    stack<char, 4> s;
    s.push('A'); // [A]
    s.push('B'); // [A,B]
    s.pop();     // [A]
    s.push('C'); // [A,C]
    s.push('D'); // [A,C,D]

    char expected[] = {'D', 'C', 'A'};
    for (auto e : expected)
      result &= (s.pop() == e);
  }

  return result;
}

TEST(template_specialization)
{
  bool result = true;

  { // Large stack with different type
    stack<double, 100> s;
    for (int i = 0; i < 100; i++)
      s.push(i * 0.1);
    for (int i = 99; i >= 0; i--)
      result &= (s.pop() == i * 0.1);
  }

  { // Boolean stack
    stack<bool, 4> s;
    s.push(true);
    s.push(false);
    s.push(true);
    s.push(true);

    result &= (s.pop() == true);
    result &= (s.pop() == true);
    result &= (s.pop() == false);
    result &= (s.pop() == true);
  }

  return result;
}

END_SUITE()
