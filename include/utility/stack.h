#pragma once

#include "core/types.h"
#include "utility/debug.h"

template <typename T, size_t N>
class stack
{
protected:
  uint8_t r_arr[sizeof(T) * N];
  size_t sz = 0;

private:
  struct iterator {
    using value_type = T;
    using pointer = value_type *;
    using reference = value_type &;

    reference operator*() const { return arr[sz - 1]; }

    pointer operator->() { return arr + sz - 1; }

    iterator &operator++()
    {
      sz--;
      return *this;
    }

    iterator operator++(int)
    {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool operator==(const iterator &lhs, const iterator &rhs)
    {
      return lhs.sz == rhs.sz;
    }

    friend bool operator!=(const iterator &lhs, const iterator &rhs)
    {
      return !(lhs == rhs);
    }

    iterator(pointer arr, size_t sz) : arr{arr}, sz{sz} {}

  private:
    const pointer arr;
    size_t sz;
  };

public:
  template <typename... Args>
  T *push(Args &&...args)
  {
    if (sz == N)
      return nullptr;

    auto t_arr = (T *)r_arr;
    auto t = new (t_arr + sz) T{std::forward<Args>(args)...};
    sz++;

    return t;
  }

  T pop()
  {
    auto t_arr = (T *)r_arr;
    return std::move(t_arr[--sz]);
  }

  T &top()
  {
    auto t_arr = (T *)r_arr;
    return t_arr[sz - 1];
  }

  bool empty() const { return sz == 0; }

  size_t size() const { return sz; }

  iterator begin() { return iterator{(T *)r_arr, sz}; }
  iterator end() { return iterator{(T *)r_arr, 0}; }
};
