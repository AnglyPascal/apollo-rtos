#pragma once

#include "core/types.h"
#include "utility/debug.h"

template <typename T, size_t N>
class stack
{
protected:
  uint8_t r_arr[sizeof(T) * N];
  size_t sz = 0;

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
};
