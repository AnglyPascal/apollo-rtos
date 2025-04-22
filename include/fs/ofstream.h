#pragma once

#include "fs/fs_impl.h"

#include "core/types.h"
#include "utility/debug.h"
#include "utility/format.h"

template <auto desc>
class _ofstream
{
  using file_t = fs_impl_t<desc>::file_t;
  file_t file;

  static constexpr size_t buf_len = 16;
  static_assert(desc.blk_sz % buf_len == 0);

  char buf[buf_len];
  size_t buf_pos = 0;

  size_t offset;

  void flush()
  {
    if (buf_pos != 0) {
      file.write(buf, buf_pos, offset);
      offset += buf_pos;
      buf_pos = 0;
    }
  }

public:
  _ofstream(fn_t fn, uint32_t flags = 0) : file{fn, flags | O_WRITE}, offset{0}
  {
    file.lock();
  }

  _ofstream(const file_t &file) : _ofstream{file.fn()} { file.lock(); }

  ~_ofstream()
  {
    flush();
    file.unlock();
  }

  void seek(size_t off) { offset = off == EOF ? file.fend() : off; }

  void putc(char c)
  {
    if (buf_pos == buf_len)
      flush();
    buf[buf_pos++] = c;
  }

  void puts(const char *str, size_t sz)
  {
    flush();
    file.write(str, sz, offset);
    offset += sz;
  }

  template <typename... Args>
  void printf(Args... args)
  {
    do_printf(*this, std::forward<Args>(args)...);
  }

  friend _ofstream &operator<<(_ofstream &os, char c)
  {
    os.putc(c);
    return os;
  }

  friend _ofstream &operator<<(_ofstream &os, const char *str)
  {
    os.printf(str);
    return os;
  }

  friend _ofstream &operator<<(_ofstream &os, int i)
  {
    os.printf("%d", i);
    return os;
  }

  friend _ofstream &operator<<(_ofstream &os, uint32_t i)
  {
    os.printf("%u", i);
    return os;
  }
};
