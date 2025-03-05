#pragma once

#include "core/fs.h"
#include "core/memory.h"
#include "drivers/fs_bck.h"
#include "utility/allocator.h"
#include "utility/debug.h"
#include "utility/mutex.h"

struct mmap_unit_t {
  void *buf = nullptr;
  size_t sz = 0;
  bool pool = false;
};

template <typename desc_t>
class _fd_t
{
  using inode_t = _inode_t<desc_t>;
  using fn_t = typename desc_t::fn_t;

  uint8_t w_cnt = 0;
  uint8_t r_cnt = 0;

public:
  fn_t fn = null_fn;
  mutex<> mtx = {"fd"};

  const inode_t *inode = nullptr;
  mmap_unit_t mu = {};

  void acquire(fn_t _fn, const inode_t *_inode, bool w_en)
  {
    assert(fn == null_fn || fn == _fn);
    r_cnt++;
    w_cnt += w_en;
    fn = _fn;
    inode = _inode;
  }

  void release(bool w_en)
  {
    r_cnt--;
    w_cnt -= w_en;
    if (r_cnt == 0)
      new (this) _fd_t{};
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

    mmap_unit_t mu;

    const bool mmap_shared = false;
    const bool w_en = false;

    mmap_unit_t &target_mu() { return mmap_shared ? fd->mu : mu; }

  public:
    file_t() {}

    file_t(fn_t fn, size_t _sz, uint32_t flags)
        : mmap_shared{(bool)(flags & O_SHARED)}, w_en{(bool)(flags & O_WRITE)}
    {
      fd = find_fd(fn);
      if (fd == nullptr) {
        debug<ERROR>("cannot find fd in open files table\r\n");
        return;
      }

      fd->acquire(fn, fs.open(fn, _sz, flags), w_en);
    }

    ~file_t() { close(); }

    void close()
    {
      unmap();
      fd->release(w_en);
    }

    fn_t fn() const { return fd->fn; }
    auto ft() const { return fd->inode->flag.ft(); }
    auto fsz() const { return fd->inode->fsz(); }

    void *mmap(size_t buf_sz)
    {
      auto &tmu = target_mu();
      if (tmu.buf != nullptr)
        return tmu.buf;

      tmu.buf = pool.alloc(buf_sz);
      tmu.sz = buf_sz;
      tmu.pool = true;

      load();
      return tmu.buf;
    }

    void mmap(uint8_t *buf, size_t buf_sz)
    {
      unmap();

      auto &tmu = target_mu();
      tmu.buf = buf;
      tmu.sz = buf_sz;
      tmu.pool = false;

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
      auto &tmu = target_mu();
      if (tmu.pool)
        pool.dealloc((byte_t *)tmu.buf);

      tmu.buf = nullptr;
      tmu.sz = 0;
      tmu.pool = false;
    }

    void load()
    {
      auto &tmu = target_mu();
      assert_dump(tmu.buf != nullptr, "%d\r\n", fd->fn);
      lock_guard guard{fd->mtx};
      fs.load(fd->inode, (uint8_t *)tmu.buf, tmu.sz);
    }

    void store()
    {
      auto &tmu = target_mu();
      assert(tmu.buf != nullptr);
      lock_guard guard{fd->mtx};
      fs.store(fd->inode, (uint8_t *)tmu.buf, tmu.sz);
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

  static void remove(fn_t fn)
  {
    if (is_open(fn)) {
      debug<ERROR>("fn %d is open, can't remove\r\n", fn);
      return;
    }
    fs.remove(fn);
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

  static bool is_open(fn_t fn)
  {
    auto fd = find_fd(fn);
    return fd != nullptr && fd->fn == fn;
  }

  static void trace() { fs.trace(is_open); }
};
