#pragma once

#include "core/types.h"
#include "drivers/fs_bck.h"
#include "utility/debug.h"

template <typename desc_t>
class _file_t;

template <typename desc_t>
struct _fd_t {
public:
  using fn_t = typename desc_t::fn_t;
  using blk_addr_t = typename desc_t::blk_addr_t;
  using nblks_t = typename desc_t::nblks_t;

  using inode_t = _inode_t<desc_t>;

  friend class _file_t<desc_t>;

  // FIXME: add reference counter and whatnot
  fn_t fn;

private:
  const inode_t *inode;
  void *mmap_buf;
  size_t mmap_buf_sz;

  file_type_t ft() const { return inode->flag.ft(); }

  nblks_t nblks() const { return inode->nblks; }
  size_t fsz() const { return inode->fsz(); }

public:
  _fd_t() : fn{desc_t::null_fn}, inode{nullptr} {}
  _fd_t(fn_t fn, const inode_t *inode) : fn{fn}, inode{inode} {}

  bool in_use() const { return inode != nullptr; }

  void acquire(fn_t _fn, const inode_t *_inode)
  {
    fn = _fn;
    inode = _inode;
  }
};

template <typename desc_t>
class _file_t
{
  using fn_t = typename desc_t::fn_t;
  using fd_t = _fd_t<desc_t>;
  using inode_t = _inode_t<desc_t>;

private:
  fd_t &fd;

public:
  const bool w_en;
  const file_type_t ft;
  size_t fsz;

public:
  _file_t(fd_t &fd, bool w_en) : fd{fd}, w_en{w_en}, ft{fd.ft()}, fsz{fd.fsz()}
  {
  }

  fn_t fn() const { return fd->fn; }

  void mmap(void *buf, size_t buf_sz)
  {
    fd.mmap_buf = buf;
    fd.mmap_buf_sz = buf_sz;
  }

  void *unmap()
  {
    auto buf = fd.mmap_buf;
    fd.mmap_buf = nullptr;
    fd.mmap_buf_sz = 0;
    return buf;
  }

  const inode_t *inode() const { return fd.inode; }
  void *mmap_buf() const { return fd.mmap_buf; }
  size_t mmap_buf_sz() const { return fd.mmap_buf_sz; }
};

