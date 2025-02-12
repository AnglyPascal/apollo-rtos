#include "core/fs.h"
#include "core/file.h"
#include "core/memory.h"
#include "drivers/flash.h"
#include "drivers/fs_bck.h"
#include "utility/allocator.h"
#include "utility/debug.h"

template <typename desc_t, size_t N_OPEN_FILES>
class open_ftbl_t
{
public:
  using fn_t = typename desc_t::fn_t;
  using fd_t = _fd_t<desc_t>;

private:
  fd_t open_files[N_OPEN_FILES] = {};

public:
  fd_t *find_fd(fn_t fn)
  {
    fd_t *empty_fd = nullptr;
    for (auto &fd : open_files) {
      if (fd.fn == fn)
        return &fd;
      else if (!fd.in_use())
        empty_fd = &fd;
    }
    return empty_fd;
  }

  void ret_fd(fd_t &fd) { fd = fd_t{}; }
};

using ft_func_t = file_type_t (*)(uint32_t);

template <typename desc_t, desc_t desc, size_t N_OPEN_FILES, ft_func_t ft_func>
class fs_impl_t
{
  using file_t = _file_t<desc_t>;

  allocator<alloc_heap, 4> pool;

  using fs_t = _fs_t<desc_t, desc>;
  fs_t fs;

  open_ftbl_t<desc_t, N_OPEN_FILES> open_ftbl;

public:
  file_t open(fn_t fn, size_t sz, uint32_t flags)
  {
    auto fd = open_ftbl.find_fd(fn);
    if (fd == nullptr)
      debug<ERROR>("cannot find fd in open files table\r\n");

    if (!fd->in_use()) {
      auto inode = fs.open(fn, sz, flags);
      fd->acquire(fn, inode);
    }

    return {*fd, (bool)(flags & O_WRITE)};
  }

  void *mmap(file_t &file, size_t buf_sz)
  {
    void *buf = file.mmap_buf();
    if (buf != nullptr)
      return buf;

    buf = pool.alloc(buf_sz);
    file.mmap(buf, buf_sz);
    load(file);

    return buf;
  }

  void unmap(file_t &file)
  {
    auto buf = file.unmap();
    pool.dealloc((byte_t *)buf);
  }

  void mount() { fs.mount(); }
  void format() { fs.format(); }

  void load(file_t &file) const
  {
    assert(file.mmap_buf() != nullptr);
    auto inode = file.inode();
    fs.load(inode, (uint8_t *)file.mmap_buf(), file.mmap_buf_sz());
  }

  void store(file_t &file) const
  {
    assert(file.mmap_buf() != nullptr);
    auto inode = file.inode();
    fs.store(inode, (uint8_t *)file.mmap_buf(), file.mmap_buf_sz());
  }
};

namespace flash
{
namespace
{
static constexpr ft_func_t ft_func = [](uint32_t flag) { return CHAR; };
fs_impl_t<desc_t, desc, 8, ft_func> fs_impl;
} // namespace

file_t open(fn_t fn, size_t sz, uint32_t flags)
{
  return fs_impl.open(fn, sz, flags);
}

void *mmap(file_t &file, size_t buf_sz) { return fs_impl.mmap(file, buf_sz); }
void unmap(file_t &file) { return fs_impl.unmap(file); }

void mount() { fs_impl.mount(); }
void format() { fs_impl.format(); }

void load(file_t &file) { fs_impl.load(file); }
void store(file_t &file) { fs_impl.store(file); }

} // namespace flash
