#pragma once

#include "core/fs.h"
#include "core/memory.h"
#include "drivers/fs_bck.h"
#include "utility/allocator.h"
#include "utility/debug.h"
#include "utility/mutex.h"

template <typename desc_t>
class _fd_t
{
  uint8_t w_cnt = 0;
  uint8_t r_cnt = 0;

public:
  using fn_t = typename desc_t::fn_t;
  fn_t fn = null_fn;
  mutex<> mtx = {"fd"};

  void acquire(fn_t _fn, bool w_en)
  {
    assert(fn == null_fn || fn == _fn);
    r_cnt++;
    w_cnt += w_en;
    fn = _fn;
  }

  void release(bool w_en)
  {
    r_cnt--;
    w_cnt -= w_en;
    if (r_cnt == 0) {
      fn = null_fn;
    }
  }
};

using ft_func_t = file_type_t (*)(uint32_t);

template <typename desc_t, desc_t desc, size_t N_OPEN_FILES, ft_func_t ft_func>
class fs_impl_t
{
  using fd_t = _fd_t<desc_t>;
  using inode_t = _inode_t<desc_t>;

  using fs_t = _fs_t<desc_t, desc>;
  inline static fs_t fs;

  inline static allocator<alloc_heap, 4> pool;
  inline static fd_t open_files[N_OPEN_FILES] = {};

  static fd_t *find_fd(fn_t fn)
  {
    fd_t *empty_fd = nullptr;
    for (auto &fd : open_files) {
      if (fd.fn == fn)
        return &fd;
      if (fd.fn == null_fn)
        empty_fd = &fd;
    }
    return empty_fd;
  }

public:
  class file_t
  {
    fd_t *fd = nullptr;

    const inode_t *inode = nullptr;

    void *mmap_buf = nullptr;
    size_t mmap_buf_sz = 0;
    bool mmap_pool = false;

    bool w_en = false;

  public:
    file_t() {}

    file_t(fn_t fn, size_t _sz, uint32_t flags)
    {
      fd = find_fd(fn);
      if (fd == nullptr) {
        debug<ERROR>("cannot find fd in open files table\r\n");
        return;
      }

      w_en = flags & O_WRITE;

      inode = fs.open(fn, _sz, flags);
      fd->acquire(fn, w_en);
    }

    ~file_t() { close(); }

    void close()
    {
      unmap();
      fd->release(w_en);
    }

    fn_t fn() const { return fd->fn; }
    auto ft() const { return inode->flag.ft(); }
    auto fsz() const { return inode->fsz(); }

    void *mmap(size_t buf_sz)
    {
      if (mmap_buf != nullptr)
        return mmap_buf;

      mmap_buf = pool.alloc(buf_sz);
      mmap_buf_sz = buf_sz;
      mmap_pool = true;

      load();
      return mmap_buf;
    }

    void mmap(uint8_t *buf, size_t buf_sz)
    {
      unmap();

      mmap_buf = buf;
      mmap_buf_sz = buf_sz;
      mmap_pool = false;

      load();
    }

    template <typename T>
    T *mmap()
    {
      return (T *)mmap(sizeof(T));
    }

    template <typename T>
    void mmap(T &obj)
    {
      mmap((uint8_t *)&obj, sizeof(T));
    }

    void unmap()
    {
      if (mmap_pool)
        pool.dealloc((byte_t *)mmap_buf);

      mmap_buf = nullptr;
      mmap_buf_sz = 0;
      mmap_pool = false;
    }

    void load()
    {
      assert(mmap_buf != nullptr);
      lock_guard guard{fd->mtx};
      fs.load(inode, (uint8_t *)mmap_buf, mmap_buf_sz);
    }

    void store()
    {
      assert(mmap_buf != nullptr);
      lock_guard guard{fd->mtx};
      fs.store(inode, (uint8_t *)mmap_buf, mmap_buf_sz);
    }
  };

public:
  static file_t open(fn_t fn, size_t sz, uint32_t flags)
  {
    return file_t{fn, sz, flags};
  }

  static void open(file_t &file, fn_t fn, size_t sz, uint32_t flags)
  {
    new (&file) file_t{fn, sz, flags};
  }

  static void mount()
  {
    fs.mount();
    for (auto &fd : open_files) {
      new (&fd) fd_t{};
    }
  }

  static void format() { fs.format(); }

  static bool first_boot() { return fs.first_boot; }

  static void trace() { fs.trace(); }
};
