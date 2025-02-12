#pragma once

#include "core/types.h"
#include "utility/debug.h"

template <typename len_t, len_t len>
  requires std::is_integral_v<len_t>
class bitset
{
public:
  struct iterator {
    using value_type = len_t;

    value_type operator*() { return idx; };

    iterator &operator++()
    {
      if (rem > 0) {
        idx = set.next(idx + 1);
        rem--;
      }
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
      return lhs.rem == rhs.rem;
    }

    friend bool operator!=(const iterator &lhs, const iterator &rhs)
    {
      return !(lhs == rhs);
    }

    iterator(const bitset &set, len_t idx, len_t rem)
        : set{set}, idx{set.next(idx)}, rem{rem}
    {
    }

  private:
    const bitset &set;
    len_t idx;
    len_t rem;
  };

private:
  inline pair<uint8_t &, uint8_t> byte_and_msk(len_t idx)
  {
    assert(idx >= 0 && idx < len, "idx: %u, len: %u\r\n", idx, len);
    return {store[idx / 8], 1 << (idx % 8)};
  }

  inline pair<const uint8_t &, uint8_t> byte_and_msk(len_t idx) const
  {
    assert(idx >= 0 && idx < len);
    return {store[idx / 8], 1 << (idx % 8)};
  }

public:
  bool insert(len_t idx)
  {
    auto [byte, msk] = byte_and_msk(idx);
    bool exists = byte & msk;
    if (!exists)
      sz++;
    byte |= msk;
    return !exists;
  }

  bool contains(len_t idx) const
  {
    auto [byte, msk] = byte_and_msk(idx);
    return byte & msk;
  }

  bool erase(len_t idx)
  {
    auto [byte, msk] = byte_and_msk(idx);
    bool exists = byte & msk;
    if (exists)
      sz--;
    byte &= (~msk);
    return exists;
  }

  len_t next(len_t idx = 0) const
  {
    while (idx < len && !contains(idx))
      idx++;
    return idx;
  }

  len_t size() const { return sz; }
  bool empty() const { return sz == 0; }

  void set_all()
  {
    uint8_t msk = -1;
    for (size_t idx = 0; idx < bytes; idx++) {
      store[idx] = msk;
    }
    sz = len;
  }

  iterator begin() const { return iterator{*this, 0, sz}; }
  iterator end() const { return iterator{*this, len, 0}; }

private:
  static constexpr len_t bytes = roundup(len, 8) / 8;

  uint8_t store[bytes] = {(uint8_t)0};
  len_t sz = 0;
};
