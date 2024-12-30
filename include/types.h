#pragma once

#include <cstdint>
#include <limits>

using size_t = std::size_t;
using runnable_t = void *(*)(void *);

template <typename T>
constexpr T _max = std::numeric_limits<T>::max();

using time_t = uint32_t;

struct string {
  const char *str;

  constexpr string() : str{0} {}
  constexpr string(const string &other) : str(other.str) {}
  constexpr string(const char _str[]) : str{_str} {}

  constexpr const string &operator=(const string &other)
  {
    str = other.str;
    return *this;
  }

  constexpr const string &operator=(const char _str[])
  {
    str = _str;
    return *this;
  }

  bool operator==(const string &other) const
  {
    auto lhs = str, rhs = other.str;

    if (!lhs && !rhs)
      return true;

    if (!lhs || !rhs)
      return false;

    while (*lhs != '\0' && *rhs != '\0' && *lhs == *rhs) {
      lhs++;
      rhs++;
    }

    return *lhs == '\0' && *rhs == '\0';
  }
};

template <typename T1, typename T2>
struct pair {
  T1 first;
  T2 second;
};

template <size_t len>
class buffer
{
  char arr[len];
  int sz = 0;

public:
  bool ready = false;

public:
  void push(char c)
  {
    if (sz == len)
      return;

    if (c == '\n' || c == '\r') {
      ready = true;
      c = '\0';
    }

    arr[sz++] = c;
  }

  bool check_suffix(const char *str) const
  {
    int i = 0;
    while (i < sz && *str != '\0' && arr[i] == *str)
      i++;
    return *str == '\0';
  }

  pair<string, string> parse()
  {
    auto cmd = arr;
    while (*cmd != ' ') {
      if (*cmd == '\0')
        return {arr, cmd};
      cmd++;
    }
    *cmd++ = '\0';
    return {arr, cmd};
  }

  void reset()
  {
    sz = 0;
    arr[0] = '\0';
    ready = false;
  }
};

template <typename T>
struct node {
  T *val = nullptr;
  node *next = nullptr;
};

template <typename T>
class list
{
  node<T> head = {nullptr, nullptr};

  class iterator
  {
    const node<T> *head;

  public:
    iterator(const node<T> *head) : head(head) {}

    T &operator*() const
    {
      return *head->next->val;
    }

    T *operator->() const
    {
      return head->next->val;
    }

    iterator operator++() const
    {
      return iterator{head->next};
    }

    bool operator==(const iterator &other) const
    {
      return head == other.head;
    }

    bool operator!=(const iterator &other) const
    {
      return head != other.head;
    }
  };

  using const_iterator = const iterator;

public:
  void push_front(node<T> *nd)
  {
    nd->next = head.next;
    head.next = nd;
  }

  iterator begin() const
  {
    return iterator{&head};
  }

  iterator end() const
  {
    return iterator{nullptr};
  }
};

template <typename T, size_t len>
class stack
{
  T arr[len];
  size_t sz = 0;

  class iterator
  {
    T *arr;

  public:
    iterator() : arr{nullptr} {}
    iterator(T *arr) : arr{arr} {}
    iterator(const iterator &other) : arr{other.arr} {}

    T &operator*() const
    {
      return *arr;
    }

    T *operator->() const
    {
      return arr;
    }

    iterator operator++()
    {
      return iterator{++arr};
    }

    iterator &operator++(int)
    {
      ++arr;
      return *this;
    }

    bool operator==(const iterator &other) const
    {
      return arr == other.arr;
    }

    bool operator!=(const iterator &other) const
    {
      return arr != other.arr;
    }
  };

public:
  bool full()
  {
    return sz == len;
  }

  bool empty()
  {
    return sz == 0;
  }

  void push(T val)
  {
    if (sz == len)
      return;
    arr[sz++] = val;
  }

  T pop()
  {
    return arr[sz--];
  }

  iterator begin()
  {
    return iterator{arr};
  }

  iterator end()
  {
    return iterator{arr + len};
  }
};
