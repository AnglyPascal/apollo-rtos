template <typename T, size_t N>
class stack
{
protected:
  uint8_t r_arr[sizeof(T) * N];
  size_t sz = 0;

private:
  // iterates over the stack top to bottom
  struct iterator {
     /* ... */
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
