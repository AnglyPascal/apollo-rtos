#pragma once

#include "types.h"

template <size_t len>
struct char_buffer {
  char str[len + 1] = {'\0'};
  size_t sz = 0;

  void push(char c)
  {
    if (c == '\0' || sz == len) {
      str[sz] = '\0';
      return;
    }
    str[sz++] = c;
  }

  void pop()
  {
    if (sz > 0)
      str[--sz] = '\0';
  }

  char_buffer<len> &operator+=(char c)
  {
    push(c);
    return *this;
  }

  bool match(const string &other) const
  {
    size_t i = 0;
    auto str = other.str;
    while (i < len && *str != '\0' && str[i] == *str) {
      i++;
      str++;
    }
    return *str == '\0';
  }

  void replace(size_t idx, char c)
  {
    str[idx] = c;
  }

  size_t find(char c) const
  {
    for (size_t i = 0; i < sz; i++) {
      if (str[i] == c)
        return i;
    }
    return sz;
  }

  pair<string, string> split(size_t i)
  {
    if (i == (size_t)~0)
      return {str, str + sz};
    return {str, str + i};
  }

  void reset()
  {
    sz = 0;
  }
};
