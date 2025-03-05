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

  static constexpr size_t N_BLKS = (desc.end - desc.start) / desc_t::blk_sz;
  static_assert(N_BLKS <= (1 << 8));
  bitset<N_BLKS> free_set;

  bool valid() const { return magic == BLK_MAGIC; }

  void format()
  {
    magic = BLK_MAGIC;
    for (auto &inode : inode_tbl) {
      inode = inode_t{};
    }
    free_set.set_all();
  }

  inode_t &find(fn_t fn)
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

  static constexpr auto BLK_SZ = desc_t::blk_sz;

  bool first_boot = false;

private:
  using fs_hd_t = _fs_hd_t<desc_t, desc>;
  fs_hd_t fs_hd;
  static_assert(sizeof(fs_hd_t) <= desc.fs_hd_sz);

  mutable mutex<N_PROCS_WAIT> w_mtx{"fs mtx"};
  mutable blk_addr_t temp_blk[desc.max_num_blks + 1];

  static constexpr addr_t paddr(blk_addr_t blk_addr)
  {
    auto addr = desc.start + (addr_t)blk_addr * BLK_SZ;
    assert(desc.start <= addr && addr < desc.end);
    return addr;
  }

  inline void store_hd() const
  {
    write(desc.fs_hd_addr, (uint8_t *)&fs_hd, sizeof(fs_hd_t));
  }

public:
  bool valid() const { return fs_hd.valid(); }

  void format()
  {
    fs_hd.format();
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
    addr_t addr = paddr(blk_addr);
    desc.write(addr, buf, buf_sz);
  }

  static inline void read(blk_addr_t blk_addr, uint8_t *buf, size_t buf_sz)
  {
    addr_t addr = paddr(blk_addr);
    desc.read(addr, buf, buf_sz);
  }

  const inode_t *open(fn_t fn, size_t sz, uint32_t flags)
  {
    assert(sz > 0 && fn >= 0 && fn < desc.n_inodes);

    auto &inode = fs_hd.find(fn);
    if (!inode.flag.in_use()) {
      assert(flags & O_CREATE, "inode doesn't exist, but not creating\r\n");
      inode.flag.set_use();

      file_type_t ft = flags & O_CHAR_FILE ? CHAR : BIN;

      nblks_t nblks = 1;
      if (sz > BLK_SZ)
        nblks += roundup(sz, BLK_SZ) / BLK_SZ;
      assert(nblks <= desc.max_num_blks + 1);

      fs_hd.alloc_blks(temp_blk, nblks);
      blk_addr_t first_blk_addr = temp_blk[0];

      if (nblks > 1)
        write(first_blk_addr, (uint8_t *)(temp_blk + 1),
              (nblks - 1) * sizeof(blk_addr_t));

      inode.addr = first_blk_addr;
      inode.nblks = nblks;
      inode.flag.set_ft(ft);

      if (flags & O_PERM)
        inode.flag.set_perm();

      inode.start = 0;
      inode.curr = 0;

      store_hd();
    }

    return &inode;
  }

  void remove(fn_t fn)
  {
    auto &inode = fs_hd.find(fn);

    if (!inode.flag.in_use()) {
      debug<WARN>("deleting non-existent file does nothing\r\n");
      return;
    }

    if (inode.flag.is_perm()) {
      debug<ERROR>("cannot delete permanent file\r\n");
      return;
    }

    auto nblks = inode.nblks;
    auto temp_blk[0] = inode.addr;
    if (nblks > 1)
      read(inode.addr, (uint8_t *)(temp_blk + 1),
           (nblks - 1) * sizeof(blk_addr_t));

    fs_hd.dealloc_blks(temp_blk, nblks);
    inode.flag.reset();

    store_hd();
  }

private:
  template <bool to_read>
  inline void xfer(const inode_t *inode, uint8_t *buf, size_t buf_sz) const
  {
    assert(inode->ft() == BIN);
    auto func = to_read ? read : write;

    auto fst_blk = inode->addr;
    auto nblks = inode->nblks;

    size_t file_sz = nblks > 1 ? (nblks - 1) * BLK_SZ : BLK_SZ;
    assert(file_sz >= buf_sz);

    if (nblks == 1) {
      func(fst_blk, buf, buf_sz);
      return;
    }

    read(fst_blk, (uint8_t *)temp_blk, (nblks - 1) * sizeof(blk_addr_t));

    for (nblks_t i = 0; i < nblks - 2; i++) {
      auto blk_addr = temp_blk[i];
      func(blk_addr, buf, BLK_SZ);

      buf += BLK_SZ;
      buf_sz -= BLK_SZ;
    }
    func(temp_blk[nblks - 2], buf, buf_sz);
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

  void trace()
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
      debug<INFO>("  |  %d: size = %d, type = %s, blks: ", fn, inode.fsz(),
                  inode.ft() == CHAR ? "char" : "bin");

      auto fst_blk = inode.addr;
      auto nblks = inode.nblks;

      if (nblks > 1) {
        read(fst_blk, temp_blk, (nblks - 1) * sizeof(blk_addr_t));
        auto blks = (blk_addr_t *)temp_blk;
        for (auto i = 0; i < nblks - 1; i++) {
          debug<INFO>("%u, ", blks[i]);
        }
      }

      debug<INFO>("%u\r\n", fst_blk);
    }
  }

  // TODO:
  void flush();
  // or smth similar to be used in hardfault
};

