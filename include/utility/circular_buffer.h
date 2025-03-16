#pragma once

#include "core/types.h"
#include "utility/debug.h"
#include <utility>

template <typename T, size_t N>
class circular_buffer
{
protected:
  byte_t store[N * sizeof(T)];
  size_t _start = 0, _end = 0, sz = 0;

private:
  struct iterator {
    using difference_type = size_t;
    using value_type = T;
    using pointer = value_type *;
    using reference = value_type &;

    reference operator*() const { return arr[i]; }

    pointer operator->() { return arr + i; }

    iterator &operator++()
    {
      i = i + 1 == N ? 0 : i + 1;
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

    iterator(pointer arr, size_t i, size_t sz) : arr{arr}, i{i}, sz{sz} {}

  private:
    const pointer arr;
    size_t i;
    size_t sz;
  };

public:
  template <typename... Args>
  T *enqueue(Args &&...args)
  {
    auto arr = (T *)store;
    auto ptr = new (arr + _end) T{std::forward<Args>(args)...};

    _end = _end + 1 == N ? 0 : _end + 1;
    if (sz == N) {
      _start = _start + 1 == N ? 0 : _start + 1;
    } else {
      sz++;
    }

    return ptr;
  }

  T dequeue()
  {
    assert(sz > 0, H_RESET);
    auto arr = (T *)store;
    auto t = arr[_start];
    _start = _start + 1 == N ? 0 : _start + 1;
    sz--;
    return t;
  }

  const T &peek() const
  {
    assert(sz > 0, H_RESET);
    auto arr = (T *)store;
    return arr[_start];
  }

  T front() const
  {
    assert(sz > 0, H_RESET);
    auto arr = (T *)store;
    return arr[_start];
  }

  T back() const
  {
    assert(sz > 0, H_RESET);
    auto i = _end == 0 ? N - 1 : _end - 1;
    auto arr = (T *)store;
    return arr[i];
  }

  bool empty() const { return sz == 0; }
  size_t size() const { return sz; }
  size_t capacity() const { return N; }

  iterator begin() { return iterator{(T *)store, _start, sz}; }
  iterator end() { return iterator{(T *)store, _end, 0}; }
};

inline constexpr size_t circular_buffer_header_sz = sizeof(size_t) * 3;
