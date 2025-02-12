#pragma once

#include "core/types.h"

template <typename addr_t>
using fs_xfer_t = void (*)(addr_t addr, uint8_t *buf, size_t buf_sz);

template <typename _fn_t, typename _blk_addr_t, typename _addr_t,
          typename _nblks_t, size_t _blk_sz>
struct fs_desc_t {
  using blk_addr_t = _blk_addr_t;
  using addr_t = _addr_t;
  using fn_t = _fn_t;
  using nblks_t = _nblks_t;

  static constexpr fn_t null_fn = MAX<fn_t>;

  fs_xfer_t<addr_t> write;
  fs_xfer_t<addr_t> read;

  static constexpr size_t blk_sz = _blk_sz;

  addr_t start;
  addr_t end;

  blk_addr_t fs_hd_addr;
  size_t fs_hd_sz;

  nblks_t max_num_blks;
  size_t n_inodes;
};
