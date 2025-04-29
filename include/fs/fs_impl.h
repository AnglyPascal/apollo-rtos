#pragma once

#include "fs/fs.h"
#include "fs/fs_bck.h"

#include "core/memory.h"
#include "core/sync.h"
#include "utility/allocator.h"
#include "utility/debug.h"

using fd_mtx_t = mutex<8>;

template <auto desc>
class fs_impl_t
{
private:
  using desc_t = std::remove_cv_t<decltype(desc)>;

  using inode_t = _inode_t<desc_t, desc>;
  using fs_t = _fs_t<desc_t, desc>;
  using fn_t = typename desc_t::fn_t;
  using blk_addr_t = typename desc_t::blk_addr_t;

  struct mmap_unit_t {
    void *buf = nullptr;
    size_t sz = 0;
  };

  struct fd_t {
    const fd_mtx_t mtx = {"fd"};

    fn_t fn = null_fn;
    const inode_t *inode = nullptr;

    mmap_unit_t mu = {};
    bool mu_pooled = false;

    uint8_t r_cnt = 0;
    uint8_t w_cnt = 0;
  };

private:
  inline static fs_t fs;
  inline static allocator<alloc_heap, 4> pool;
  inline static fd_t open_files[desc.n_open_files] = {};

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

    mmap_unit_t mu = {};
    bool mu_pooled = false;

    const bool mmap_shared = false;

    const bool w_en = false;
    const bool a_en = false;

    mmap_unit_t &target_mu() { return mmap_shared ? fd->mu : mu; }
    const mmap_unit_t &target_mu() const { return mmap_shared ? fd->mu : mu; }
    bool &target_mu_pooled() { return mmap_shared ? fd->mu_pooled : mu_pooled; }

  public:
    const bool new_file = false;

    file_t() {}

    file_t(fn_t fn, uint32_t flags) : file_t{fn, 0, flags} {}

    file_t(fn_t fn, size_t sz, uint32_t flags)
        : mmap_shared{(bool)(flags & O_SHARED)},
          w_en{(bool)(flags & (O_WRITE | O_APPEND))},
          a_en{(bool)(flags & O_APPEND)}, new_file{false}
    {
      fd = find_fd(fn);
      assert(fd != nullptr, TERM, "cannot find fd in open files table\r\n");

      auto [inode, _new_file] = fs.open(fn, sz, flags);
      assert(inode != nullptr, TERM, "could not open inode\r\n");

      if (fd->fn == fn) {
        assert(!_new_file && fd->inode == inode, H_RESET);

        fd->r_cnt++;
        fd->w_cnt += w_en;
        return;
      }

      const_cast<bool &>(new_file) = _new_file;

      // newly acquired fd
      fd->fn = fn;
      fd->inode = inode;

      fd->mu = mmap_unit_t{};
      fd->r_cnt = 1;
      fd->w_cnt = w_en;
    }

    ~file_t()
    {
      if (fd != nullptr)
        close();
    }

    void close()
    {
      unmap();

      fd->r_cnt--;
      fd->w_cnt -= w_en;

      fs.close(fd->inode);

      // last reference
      if (fd->r_cnt == 0) {
        fd->fn = null_fn;
        fd->inode = nullptr;
        fd->mu = mmap_unit_t{};
      }

      fd = nullptr;
    }

    void *mmap(size_t sz)
    {
      assert_dump(sz <= mfsz(), H_RESET);

      auto &tmu = target_mu();
      auto &tmu_pooled = target_mu_pooled();

      if (tmu.buf != nullptr)
        return tmu.buf;

      if (fd->inode->is_valid())
        sz = min(sz, fsz());

      tmu.buf = mmap_shared ? pool.alloc(sz) : heap::malloc(sz);
      tmu.sz = sz;
      tmu_pooled = true;

      if (fd->inode->is_valid())
        load();

      return tmu.buf;
    }

    template <typename T, typename... Args>
      requires std::constructible_from<T, Args...>
    T *mmap(Args &&...args)
    {
      auto t = mmap(sizeof(T));
      if (!fd->inode->is_valid())
        return new (t) T{std::forward<Args>(args)...};
      return (T *)t;
    }

    template <typename T>
    void mmap(T &obj)
    {
      auto sz = sizeof(T);
      assert(sz <= mfsz(), H_RESET, "obj size exceeds max file size %d\r\n",
             fn);

      unmap();

      auto &tmu = target_mu();
      auto &tmu_pooled = target_mu_pooled();

      tmu.buf = (uint8_t *)&obj;
      tmu.sz = sz;
      tmu_pooled = false;

      if (fd->inode->is_valid())
        load();
    }

    void unmap()
    {
      if (!mmap_shared) {
        if (mu_pooled)
          heap::free(mu.buf);
        new (&mu) mmap_unit_t{};
        return;
      }

      // last reference to fd
      if (fd->r_cnt == 1) {
        if (fd->mu_pooled)
          pool.dealloc((byte_t *)fd->mu.buf);
        new (&fd->mu) mmap_unit_t{};
      }
    }

    fn_t fn() const { return fd->fn; }
    auto ft() const { return fd->inode->ft(); }

    auto fsz() const { return fd->inode->end; }
    auto mfsz() const { return fd->inode->max_sz(); }
    auto fend() const { return fd->inode->end; }

    template <bool sync = ASYNC>
    bool load()
    {
      auto &tmu = target_mu();
      assert(tmu.buf != nullptr, TERM, "%d\r\n", fd->fn);

      if constexpr (sync == SYNC)
        return fs.template load<SYNC>(fd->inode, tmu.buf, tmu.sz);

      lock_guard guard{fd->mtx};
      return fs.template load<ASYNC>(fd->inode, tmu.buf, tmu.sz);
    }

    template <bool sync = ASYNC>
    size_t store() const
    {
      assert(w_en, TERM);

      auto &tmu = target_mu();
      assert(tmu.buf != nullptr, TERM);

      if constexpr (sync == SYNC)
        return fs.template store<SYNC>(fd->inode, tmu.buf, tmu.sz);

      lock_guard guard{fd->mtx};
      return fs.template store<ASYNC>(fd->inode, tmu.buf, tmu.sz);
    }

    template <bool sync = ASYNC>
    size_t write(const void *buf, size_t sz, size_t off = 0)
    {
      assert(w_en, TERM);
      assert(ft() == CHAR, TERM);

      if constexpr (sync == SYNC)
        return fs.template store<SYNC>(fd->inode, buf, sz, off);

      lock_guard guard{fd->mtx};
      return fs.template store<ASYNC>(fd->inode, buf, sz, off);
    }

    template <bool sync = ASYNC>
    size_t append(const void *buf, size_t sz)
    {
      assert(w_en, TERM);
      assert(ft() == CHAR, TERM);

      if constexpr (sync == SYNC)
        return fs.template store<SYNC>(fd->inode, buf, sz, fend());

      lock_guard guard{fd->mtx};
      return fs.template store<ASYNC>(fd->inode, buf, sz, fend());
    }

    template <bool sync = ASYNC>
    size_t read(uint8_t *buf, size_t len, size_t off = 0) const
    {
      assert(ft() == CHAR, TERM);

      if constexpr (sync == SYNC)
        return fs.template load<SYNC>(fd->inode, buf, len, off);

      lock_guard guard{fd->mtx};
      return fs.template load<ASYNC>(fd->inode, buf, len, off);
    }
  };

public:
  static file_t open(fn_t fn, uint32_t flags) { return file_t{fn, 0, flags}; }

  static file_t open(fn_t fn, size_t sz, uint32_t flags)
  {
    return file_t{fn, sz, flags};
  }

  template <typename T>
  static file_t open(fn_t fn, uint32_t flags)
  {
    return file_t{fn, sizeof(T), flags};
  }

  static void open(file_t &file, fn_t fn, size_t sz, uint32_t flags)
  {
    new (&file) file_t{fn, sz, flags};
  }

  static bool remove(fn_t fn, bool forced = false)
  {
    if (is_open(fn)) {
      debug<ERROR>("fn %d is open, can't remove\r\n", fn);
      return false;
    }
    return fs.remove(fn, forced);
  }

  static void mount()
  {
    pool.init_list();
    fs.mount();
    for (auto &fd : open_files) {
      new (&fd) fd_t{};
    }
  }

  static void format() { fs.format(); }

  static void umount() { fs.umount(); }

  static inline bool first_boot() { return fs.first_boot; }

  static bool is_open(fn_t fn)
  {
    auto fd = find_fd(fn);
    return fd != nullptr && fd->fn == fn;
  }

  static bool exists(fn_t fn) { return fs.hd.tbl[fn].in_use(); }

  static void trace(fn_t fn = null_fn)
  {
    if (fn == null_fn)
      fs.trace();

    auto dump = [&](fn_t fn) {
      auto &inode = fs.hd.tbl[fn];
      debug<INFO>(BOLD YELLOW "%d" DEFAULT ": "                 //
                              "size = " BLUE "%d" DEFAULT ", "  //
                              "type = " YELLOW "%s" DEFAULT " " //
                              "blks: ",
                  fn, inode.end, inode.ft() == CHAR ? "char," : "bin, ");

      for (auto i = 0; i < inode.nblks - 1; i++)
        debug<INFO>("%u, ", inode.blks[i]);
      debug<INFO>("%u\r\n", inode.blks[inode.nblks - 1]);
    };

    if (fn != null_fn) {
      if (!exists(fn))
        return debug<INFO>("file " YELLOW "%d" DEFAULT " not found\r\n", fn);

      debug<INFO>("[" BLUE "%c" DEFAULT "] ", is_open(fn) ? 'O' : 'C');
      return dump(fn);
    }

    debug<INFO>(GREEN "  open files:" DEFAULT "\r\n");
    for (fn_t fn = 0; fn < desc.n_inodes; fn++) {
      if (exists(fn) && is_open(fn)) {
        kprintf(TAB);
        dump(fn);
      }
    }

    debug<INFO>(CYAN "  closed files:" DEFAULT "\r\n");
    for (fn_t fn = 0; fn < desc.n_inodes; fn++) {
      if (exists(fn) && !is_open(fn)) {
        kprintf(TAB);
        dump(fn);
      }
    }
  }
};
