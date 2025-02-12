#pragma once

#include "core/types.h"
#include "utility/bitset.h"
#include "utility/debug.h"
#include "utility/mutex.h"

enum file_type_t {
  CHAR = 1 << 0,
  BIN = 1 << 1,
};

struct flag_t {
  bool in_use() const { return _flag & IN_USE; }
  void set_use() { _flag |= IN_USE; }
  void unset_use() { _flag &= ~IN_USE; }

  file_type_t ft() const { return (_flag & IS_CHAR) ? CHAR : BIN; }
  void set_ft(file_type_t _ft)
  {
    if (_ft == CHAR)
      _flag |= IS_CHAR;
    else
      _flag &= ~IS_CHAR;
  }

private:
  uint8_t _flag;
  enum {
    IN_USE = 1 << 0,
    IS_CHAR = 1 << 1,
  };
};

template <typename desc_t>
struct _inode_t {
  using blk_addr_t = typename desc_t::blk_addr_t;
  using addr_t = typename desc_t::addr_t;
  using nblks_t = typename desc_t::nblks_t;

  blk_addr_t addr;
  nblks_t nblks;
  flag_t flag;

  addr_t start;
  addr_t curr;

  file_type_t ft() const { return flag.ft(); }
  size_t fsz() const
  {
    return nblks == 1 ? desc_t::blk_sz : (nblks - 1) * desc_t::blk_sz;
  }
};

template <typename desc_t, desc_t desc>
struct _fs_hd_t {
  using fn_t = typename desc_t::fn_t;
  using blk_addr_t = typename desc_t::blk_addr_t;

  using inode_t = _inode_t<desc_t>;

  static constexpr uint32_t BLK_MAGIC = 0xbabebabe;
  uint32_t magic;
  inode_t inode_tbl[desc.n_inodes];

  static constexpr blk_addr_t NBLKS = (desc.end - desc.start) / desc_t::blk_sz;
  using free_set_t = bitset<blk_addr_t, NBLKS>;
  free_set_t free_set;

  bool valid() const { return magic == BLK_MAGIC; }

  void format()
  {
    magic = BLK_MAGIC;
    for (auto &inode : inode_tbl) {
      inode = inode_t{};
    }
    free_set.set_all();
  }

  inode_t &open(fn_t fn)
  {
    auto &inode = inode_tbl[fn];
    return inode;
  }

  void alloc_blks(blk_addr_t *buf, size_t nblks)
  {
    blk_addr_t addr = free_set.next();
    while (nblks-- > 0) {
      *buf++ = addr;
      free_set.erase(addr);
      addr = free_set.next(addr + 1);
    }
  }

  void dealloc_blks(blk_addr_t *buf, size_t nblks)
  {
    while (nblks-- > 0) {
      free_set.insert(*buf++);
    }
  }
};

template <typename desc_t, desc_t desc>
  requires(desc.start < desc.end) &&
          (desc.fs_hd_addr < desc.end / desc_t::blk_sz) &&
          (desc.fs_hd_addr >= desc.start / desc_t::blk_sz) &&
          (desc.fs_hd_addr * desc_t::blk_sz + desc.fs_hd_sz <= desc.end)
class _fs_t
{
public:
  using blk_addr_t = typename desc_t::blk_addr_t;
  using addr_t = typename desc_t::addr_t;
  using fn_t = typename desc_t::fn_t;
  using nblks_t = typename desc_t::nblks_t;

  using inode_t = _inode_t<desc_t>;

  static constexpr auto blk_sz = desc_t::blk_sz;

private:
  using fs_hd_t = _fs_hd_t<desc_t, desc>;
  fs_hd_t fs_hd;
  static_assert(sizeof(fs_hd_t) <= desc.fs_hd_sz);

  mutable mutex<N_PROCS_WAIT> w_mtx;
  mutable uint8_t temp_blk[(desc.max_num_blks + 1) * sizeof(blk_addr_t)];

  static constexpr addr_t paddr(blk_addr_t blk_addr)
  {
    auto addr = desc.start + (addr_t)blk_addr * blk_sz;
    assert(desc.start <= addr && addr < desc.end);
    return addr;
  }

public:
  bool valid() { return fs_hd.valid(); }

  void format()
  {
    fs_hd.format();
    write(desc.fs_hd_addr, (uint8_t *)&fs_hd, sizeof(fs_hd_t));
  }

  void mount()
  {
    if (!valid())
      read(desc.fs_hd_addr, (uint8_t *)&fs_hd, sizeof(fs_hd_t));
    if (!valid())
      format();
  }

  void umount() { write(desc.fs_hd_addr, (uint8_t *)&fs_hd, sizeof(fs_hd_t)); }

  static void write(blk_addr_t blk_addr, uint8_t *buf, size_t buf_sz)
  {
    addr_t addr = paddr(blk_addr);
    desc.write(addr, buf, buf_sz);
  }

  static void read(blk_addr_t blk_addr, uint8_t *buf, size_t buf_sz)
  {
    addr_t addr = paddr(blk_addr);
    desc.read(addr, buf, buf_sz);
  }

  const inode_t *open(fn_t fn, size_t sz, uint32_t flags)
  {
    assert(sz > 0 && fn >= 0 && fn < desc.n_inodes);

    lock_guard guard{w_mtx};

    auto &inode = fs_hd.open(fn);
    if (!inode.flag.in_use()) {
      assert(flags & O_CREATE, "inode doesn't exist, but not creating\r\n");
      inode.flag.set_use();

      file_type_t ft = flags & O_CHAR_FILE ? CHAR : BIN;

      nblks_t nblks = 1;
      if (sz > blk_sz)
        nblks += roundup(sz, blk_sz) / blk_sz;
      assert(nblks <= desc.max_num_blks + 1);

      blk_addr_t *blk = (blk_addr_t *)temp_blk;
      fs_hd.alloc_blks(blk, nblks);
      blk_addr_t first_blk_addr = blk[0];

      if (nblks > 1)
        write(first_blk_addr, blk + 1, (nblks - 1) * sizeof(blk_addr_t));

      inode.addr = first_blk_addr;
      inode.nblks = nblks;
      inode.flag.set_ft(ft);

      inode.start = 0;
      inode.curr = 0;
    }

    return &inode;
  }

private:
  template <bool to_read>
  inline void xfer(const inode_t *inode, uint8_t *buf, size_t buf_sz) const
  {
    assert(inode->ft() == BIN);
    auto func = to_read ? read : write;

    lock_guard guard{w_mtx};

    auto fst_blk = inode->addr;
    auto nblks = inode->nblks;

    size_t file_sz = nblks > 1 ? (nblks - 1) * blk_sz : blk_sz;
    assert(file_sz >= buf_sz);

    if (nblks == 1) {
      func(fst_blk, buf, buf_sz);
      return;
    }

    read(fst_blk, temp_blk, (nblks - 1) * sizeof(blk_addr_t));
    blk_addr_t *blk = (blk_addr_t *)temp_blk;

    for (nblks_t i = 0; i < nblks - 2; i++) {
      auto blk_addr = blk[i];
      func(blk_addr, buf, blk_sz);

      buf += blk_sz;
      buf_sz -= blk_sz;
    }
    func(blk[nblks - 2], buf, buf_sz);
  }

public:
  void load(const inode_t *inode, uint8_t *buf, size_t buf_sz) const
  {
    xfer<true>(inode, buf, buf_sz);
  }

  // TODO: try storing for real
  void store(const inode_t *inode, uint8_t *buf, size_t buf_sz) const
  {
    xfer<false>(inode, buf, buf_sz);
  }

  // TODO:
  // also using a call to free_set.size(), determine free space
  void trace();

  // TODO:
  void flush();
  // or smth similar to be used in hardfault
};

