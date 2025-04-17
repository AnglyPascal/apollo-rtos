#pragma once

#include "core/types.h"
#include "utility/bitset.h"
#include "utility/debug.h"
#include "utility/mutex.h"

using fd_mtx_t = mutex<4>;

struct flag_t {
  void reset() { _flag = 0; }

  bool in_use() const { return _flag & IN_USE; }
  void set_use() { _flag |= IN_USE; }

  file_type_t ft() const { return (_flag & IS_CHAR) ? CHAR : BIN; }
  void set_ft(file_type_t _ft)
  {
    if (_ft == CHAR)
      _flag |= IS_CHAR;
    else
      _flag &= ~IS_CHAR;
  }

  bool is_perm() const { return _flag & IS_PERM; }
  void set_perm() { _flag |= IS_PERM; }

private:
  uint8_t _flag = 0;
  enum {
    IN_USE = 1 << 0,
    IS_CHAR = 1 << 1,
    IS_PERM = 1 << 2,
  };
};

template <typename desc_t, desc_t desc>
struct _inode_t {
  using blk_addr_t = typename desc_t::blk_addr_t;
  using addr_t = typename desc_t::addr_t;
  using nblks_t = typename desc_t::nblks_t;

  nblks_t nblks;
  blk_addr_t blks[desc.max_num_blks];

  flag_t flag;

  mutable addr_t curr;

  file_type_t ft() const { return flag.ft(); }
  size_t fsz() const { return nblks * desc_t::blk_sz; }
};

template <typename desc_t, desc_t desc>
struct _fs_hd_t {
  using fn_t = typename desc_t::fn_t;
  using blk_addr_t = typename desc_t::blk_addr_t;

  using inode_t = _inode_t<desc_t, desc>;

  inode_t inode_tbl[desc.n_inodes];
  uint32_t magic;

  static constexpr size_t N_BLKS = (desc.end - desc.start) / desc_t::blk_sz;
  static_assert(N_BLKS <= (1 << 8));
  bitset<N_BLKS> free_set;

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
    while (nblks-- > 0)
      free_set.insert(*buf++);
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

  using inode_t = _inode_t<desc_t, desc>;

  static constexpr auto BLK_SZ = desc_t::blk_sz;
  static constexpr uint32_t BLK_MAGIC = 0xbabebabe;

  bool first_boot = false;

private:
  using fs_hd_t = _fs_hd_t<desc_t, desc>;
  fs_hd_t fs_hd;
  static_assert(sizeof(fs_hd_t) <= desc.fs_hd_sz);

  static constexpr addr_t paddr(blk_addr_t blk_addr)
  {
    auto addr = desc.start + (addr_t)blk_addr * BLK_SZ;
    assert(desc.start <= addr && addr < desc.end, H_RESET);
    return addr;
  }

  inline void store_hd() const
  {
    write(desc.fs_hd_addr, (uint8_t *)&fs_hd, sizeof(fs_hd_t));
  }

public:
  bool valid() const { return fs_hd.magic == BLK_MAGIC; }

  void format()
  {
    fs_hd.magic = BLK_MAGIC;
    for (auto &inode : fs_hd.inode_tbl)
      inode = inode_t{};
    fs_hd.free_set.set_all();

    store_hd();
  }

  void mount()
  {
    read(desc.fs_hd_addr, (uint8_t *)&fs_hd, sizeof(fs_hd_t));
    first_boot = !valid();
    if (!valid())
      format();
  }

  void umount() const { store_hd(); }

  static inline void write(blk_addr_t blk_addr, uint8_t *buf, size_t buf_sz)
  {
    desc.write(paddr(blk_addr), buf, buf_sz);
  }

  static inline void read(blk_addr_t blk_addr, uint8_t *buf, size_t buf_sz)
  {
    desc.read(paddr(blk_addr), buf, buf_sz);
  }

  std::pair<const inode_t *, bool> open(fn_t fn, size_t sz, uint32_t flags)
  {
    assert(sz > 0 && fn >= 0 && fn < desc.n_inodes, TERM);

    auto &inode = fs_hd.inode_tbl[fn];
    const bool in_use = inode.flag.in_use();

    if (!in_use) {
      assert(flags & O_CREATE, TERM,
             "inode doesn't exist, but not creating\r\n");
      inode.flag.set_use();

      nblks_t nblks = roundup(sz, BLK_SZ) / BLK_SZ;
      assert(nblks <= desc.max_num_blks, TERM);
      inode.nblks = nblks;

      fs_hd.alloc_blks(inode.blks, nblks);

      if constexpr (desc.is_ram)
        inode.flag.set_ft(flags & O_CHAR_FILE ? CHAR : BIN);
      else
        inode.flag.set_ft(BIN);

      if (flags & O_PERM)
        inode.flag.set_perm();

      inode.curr = 0;

      store_hd();
    }

    return {&inode, !in_use};
  }

  void remove(fn_t fn)
  {
    auto &inode = fs_hd.inode_tbl[fn];

    if (!inode.flag.in_use())
      return debug<WARN>("deleting non-existent file does nothing\r\n");

    if (inode.flag.is_perm())
      return debug<ERROR>("cannot delete permanent file\r\n");

    fs_hd.dealloc_blks(inode.blks, inode.nblks);
    inode.flag.reset();

    store_hd();
  }

private:
  template <bool to_read>
  inline void xfer(const inode_t *inode, uint8_t *buf, size_t buf_sz) const
  {
    assert(inode->fsz() >= buf_sz, TERM);

    auto func = to_read ? read : write;
    auto nblks = inode->nblks;

    for (nblks_t i = 0; i < nblks - 1; i++) {
      func(inode->blks[i], buf, BLK_SZ);

      buf += BLK_SZ;
      buf_sz -= BLK_SZ;
    }
    func(inode->blks[nblks - 1], buf, buf_sz);
  }

public:
  void load(const inode_t *inode, uint8_t *buf, size_t buf_sz) const
  {
    xfer<true>(inode, buf, buf_sz);
  }

  void store(const inode_t *inode, uint8_t *buf, size_t buf_sz) const
  {
    xfer<false>(inode, buf, buf_sz);
  }

  void store(const inode_t *inode, size_t off, uint8_t *buf,
             size_t buf_sz) const
  {
    assert(off >= 0 && off < inode->fsz(), TERM);

    while (buf_sz > 0) {
      auto blk_id = off / BLK_SZ;
      auto blk_off = off % BLK_SZ;

      auto blk = inode->blks[blk_id];
      auto addr = paddr(blk) + blk_off;
      auto sz = min(buf_sz, BLK_SZ - blk_off);

      desc.write(addr, buf, sz);

      buf_sz -= sz;
      off += sz;
      buf += sz;
    }
  }

  void trace(bool (*is_open)(fn_t))
  {
    auto free_blks = fs_hd.free_set.size();
    auto free_sz = (size_t)free_blks * BLK_SZ;
    auto used_sz = fs_hd_t::N_BLKS * BLK_SZ - free_sz;
    debug<INFO>("  |  free_blks: %d, free: %d, in use: %d\r\n", free_blks,
                free_sz, used_sz);

    for (fn_t fn = 0; fn < desc.n_inodes; fn++) {
      auto &inode = fs_hd.inode_tbl[fn];
      if (!inode.flag.in_use())
        continue;
      debug<INFO>(
          "  |  [%c] %d: size = %d, type = %s, blks: ", is_open(fn) ? 'O' : 'C',
          fn, inode.fsz(), inode.ft() == CHAR ? "char" : "bin");

      for (auto i = 0; i < inode.nblks - 1; i++)
        debug<INFO>("%u, ", inode.blks[i]);
      debug<INFO>("%u\r\n", inode.blks[inode.nblks - 1]);
    }
  }
};

