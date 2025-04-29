template <typename _T>
struct node_t : public _T {
  using T = _T;

  node_t *next;
  node_t *prev;

  node_t()
    requires(std::is_default_constructible_v<T>)
      : T{}, next{nullptr}, prev{nullptr}
  {
  }

  template <typename... Args>
    requires(std::is_constructible_v<T, Args...>)
  node_t(Args &&...args)
      : T{std::forward<Args>(args)...}, next{nullptr}, prev{nullptr}
  {
  }

  void detach()
  {
    if (prev)
      prev->next = next;
    if (next)
      next->prev = prev;

    next = nullptr;
    prev = nullptr;
  }
};

template <typename _node_t>
  requires requires { typename _node_t::T; } &&
           std::same_as<node_t<typename _node_t::T>, _node_t>
class static_list_t
{
private:
  using T = typename _node_t::T;

  _node_t head;
  _node_t tail;

public:
  static_list_t()
    requires(std::is_default_constructible_v<T>)
      : head{}, tail{}
  {
    head.next = &tail;
    tail.prev = &head;
  }

  template <typename... Args>
    requires(std::is_constructible_v<T, Args...>)
  static_list_t(Args &&...args)
      : head{std::forward<Args>(args)...}, tail{std::forward<Args>(args)...}
  {
    head.next = &tail;
    tail.prev = &head;
  }

  void init()
  {
    new (&head) _node_t{&tail, nullptr};
    new (&tail) _node_t{nullptr, &head};
  }

  void push_front(_node_t *node)
  {
    node->next = head.next;
    node->prev = &head;

    head.next->prev = node;
    head.next = node;
  }

  void push_back(_node_t *node)
  {
    node->prev = tail.prev;
    node->next = &tail;

    tail.prev->next = node;
    tail.prev = node;
  }

  _node_t *front() { return head.next; }
  _node_t *back() { return tail.prev; }
  bool empty() { return head.next == &tail; }

private:
  struct iterator {
  public:
    using value_type = _node_t;
    using pointer = value_type *;
    using reference = value_type &;

  public:
    reference operator*() const { return *node; }
    pointer operator->() const { return node; }

    friend bool operator==(const iterator &lhs, const iterator &rhs)
    {
      return lhs.node == rhs.node;
    }

    friend bool operator!=(const iterator &lhs, const iterator &rhs)
    {
      return !(lhs == rhs);
    }

    iterator(_node_t *node) : node{node} {}

    iterator &operator++()
    {
      this->node = this->node->next;
      return *this;
    }

    iterator operator++(int)
    {
      auto tmp = *this;
      ++(*this);
      return tmp;
    }

  private:
    pointer node;
  };

public:
  iterator begin() { return iterator{head.next}; }
  iterator end() { return iterator{&tail}; }
};
