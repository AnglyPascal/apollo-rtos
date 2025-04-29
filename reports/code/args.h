struct args_t {
  char str[args_len];
  mutable char *s;
  bool run_bg;

  args_t(bool run_bg) : s{str}, run_bg{run_bg} {};

  char get_option() const
  {
    /* parse the next option with the format [-c] and return the character */
  }

  template <typename T = uint32_t>
    requires std::is_unsigned_v<T>
  std::optional<T> get_uint() const
  {
    /* parse the next unsigned integer type, returning nullopt if unsuccessful */
  }

  template <typename T = int32_t>
    requires std::is_signed_v<T>
  std::optional<T> get_int() const
  {
    /* parse the next signed integer type, returning nullopt if unsuccessful */
  }

  const char *get_word() const
  {
    /* parse the next word */
  }
};

