#pragma once

#include "fs/fs_impl.h"

#include "core/types.h"
#include "utility/debug.h"

template <auto desc>
class _ifstream
{
  using file_t = fs_impl_t<desc>::file_t;
  file_t file;

  static constexpr size_t buf_len = 16;
  static_assert(desc.blk_sz % buf_len == 0);

  char buf[buf_len];
  size_t buf_pos = buf_len;

  size_t remaining;
  size_t idx;
  size_t offset;

  inline void fetch()
  {
    size_t n_chars = min(remaining, buf_len);

    file.read((uint8_t *)buf, n_chars, offset);

    remaining -= n_chars;
    offset += n_chars;
    buf_pos = 0;
  }

public:
  _ifstream(fn_t fn)
      : file{fn, O_READ}, remaining{file.fsz()}, idx{remaining}, offset{0}
  {
    file.lock();
  }

  _ifstream(fn_t fn, size_t sz, size_t off = 0)
      : file{fn, O_READ}, remaining{sz}, idx{sz}, offset{off}
  {
    file.lock();
  }

  _ifstream(const file_t &file) : _ifstream{file.fn()} { file.lock(); }

  ~_ifstream() { file.unlock(); }

  char operator*()
  {
    if (idx == 0)
      return '\0';

    if (buf_pos == buf_len)
      fetch();

    return buf[buf_pos];
  }

  _ifstream &operator++()
  {
    if (buf_pos == buf_len)
      fetch();

    idx--;
    buf_pos++;
    return *this;
  }
};
