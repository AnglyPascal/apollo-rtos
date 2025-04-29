template <size_t len>
class bitset
{
  inline pair<uint8_t &, uint8_t> byte_and_msk(size_t idx) {
    return {store[idx / 8], 1 << (idx % 8)};
  }

  inline pair<const uint8_t &, uint8_t> byte_and_msk(size_t idx) const {
    return {store[idx / 8], 1 << (idx % 8)};
  }

public:
  bool insert(size_t idx)
  {
    auto [byte, msk] = byte_and_msk(idx);
    bool exists = byte & msk;
    if (!exists)
      sz++;
    byte |= msk;
    return !exists;
  }

  bool contains(size_t idx) const
  {
    auto [byte, msk] = byte_and_msk(idx);
    return byte & msk;
  }

  bool erase(size_t idx)
  {
    auto [byte, msk] = byte_and_msk(idx);
    bool exists = byte & msk;
    if (exists)
      sz--;
    byte &= (~msk);
    return exists;
  }

  size_t next(size_t idx = 0) const
  {
    if (sz == 0)
      return MAX<size_t>;

    while (idx < len && !contains(idx))
      idx++;
    return idx;
  }

  size_t size() const { return sz; }
  bool empty() const { return sz == 0; }

  void set_all()
  {
    uint8_t msk = 0xFF;
    for (size_t idx = 0; idx < bytes; idx++) {
      store[idx] = msk;
    }
    sz = len;
  }

private:
  static constexpr size_t bytes = roundup(len, 8) / 8;

  uint8_t store[bytes] = {(uint8_t)0};
  size_t sz = 0;
};
