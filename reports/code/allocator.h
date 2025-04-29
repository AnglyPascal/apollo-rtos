struct __sz {
  size_t sz;
  __sz() : sz{0} {}
};

using chunk_t = node_t<__sz>;
using chunk_list_t = static_list_t<chunk_t>;

using allocator_t = byte_t *(*)(size_t);

template <allocator_t alloc_func, size_t alignment>
class allocator
{
  chunk_list_t freelist{}; // doubly linked list

public:
  byte_t *alloc(size_t sz)
  {
    if (sz == 0)
      return nullptr;

    intr_guard guard;

    sz = roundup(sz, alignment);
    auto chnk_sz = sizeof(chunk_t) + sz;

    chunk_t *chunk = nullptr;
    for (auto it = freelist.begin(); it != freelist.end(); ++it) {
      if (it->sz >= sz && it->sz < chunk->sz)
        chunk = &*it;
    }

    if (chunk == nullptr) {
      chunk = (chunk_t *)alloc_func(chnk_sz);
      chunk->sz = sz;
    } else {
      chunk->detach();
    }

    return (byte_t *)chunk + sizeof(chunk_t);
  }

  void dealloc(byte_t *ptr)
  {
    if (ptr == nullptr)
      return;

    intr_guard guard;
    auto chunk = (chunk_t *)(ptr - sizeof(chunk_t));
    freelist.push_back(chunk);
  }
};
