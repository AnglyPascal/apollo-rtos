#pragma once

#include "core/types.h"
#include "utility/bitset.h"
#include "utility/debug.h"

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

  blk_addr_t blks[desc.max_num_blks];

  uint8_t nblks;
  flag_t flag;

  mutable addr_t end;

  file_type_t ft() const { return flag.ft(); }
  size_t fsz() const { return end; }
  size_t max_sz() const { return nblks * desc_t::blk_sz; }
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

  bool alloc_blks(blk_addr_t *buf, uint8_t nblks)
  {
    blk_addr_t addr = 0;
    while (nblks-- > 0) {
      addr = free_set.next(addr);
      if (addr == MAX<size_t>)
        return false;

      *buf++ = addr;
      free_set.erase(addr);
      addr++;
    }
    return true;
  }

  void dealloc_blks(blk_addr_t *buf, uint8_t nblks)
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

  using inode_t = _inode_t<desc_t, desc>;

  static constexpr auto BLK_SZ = desc_t::blk_sz;
  static constexpr uint32_t BLK_MAGIC = 0xbebebabe;

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

  inline void load_hd() const
  {
    desc.read(paddr(desc.fs_hd_addr), (uint8_t *)&fs_hd, sizeof(fs_hd_t));
  }

  inline void store_hd() const
  {
    desc.write(paddr(desc.fs_hd_addr), (uint8_t *)&fs_hd, sizeof(fs_hd_t));
  }

public:
  bool valid() const { return fs_hd.magic == BLK_MAGIC; }

  void format()
  {
    fs_hd.magic = BLK_MAGIC;
    for (auto &inode : fs_hd.inode_tbl)
      new (&inode) inode_t{};
    fs_hd.free_set.set_all();

    store_hd();
  }

  void mount()
  {
    load_hd();
    first_boot = !valid();
    if (!valid())
      format();
  }

  void umount() const { store_hd(); }

  std::pair<const inode_t *, bool> open(fn_t fn, size_t sz, uint32_t flags)
  {
    assert(fn >= 0 && fn < desc.n_inodes, TERM);

    auto &inode = fs_hd.inode_tbl[fn];
    const bool in_use = inode.flag.in_use();

    const bool to_create = flags & O_CREATE;

    if (in_use || !to_create) {
      assert(in_use, TERM, "file does not exist\r\n");
      if (sz != 0)
        debug<TRACE>("passing size (%d) to open existing file %d\r\n", sz, fn);
      return {&inode, false};
    }

    assert(to_create, TERM, "not creating non-existent file %d\r\n", fn);
    inode.flag.set_use();

    uint8_t nblks = roundup(max(sz, BLK_SZ), BLK_SZ) / BLK_SZ;
    assert(nblks <= desc.max_num_blks, TERM);
    inode.nblks = nblks;

    auto success = fs_hd.alloc_blks(inode.blks, nblks);
    assert(success, TERM, "block allocation failed for file %d\r\n", fn);

    if constexpr (desc.is_ram)
      inode.flag.set_ft(flags & O_CHAR_FILE ? CHAR : BIN);
    else
      inode.flag.set_ft(BIN);

    if (flags & O_PERM)
      inode.flag.set_perm();

    inode.end = 0;
    store_hd();

    return {&inode, true};
  }

  void remove(fn_t fn)
  {
    auto &inode = fs_hd.inode_tbl[fn];

    if (!inode.flag.in_use())
      return debug<WARN>("deleting non-existent file does nothing %d\r\n", fn);

    if (inode.flag.is_perm())
      return debug<ERROR>("cannot delete permanent file %d\r\n", fn);

    fs_hd.dealloc_blks(inode.blks, inode.nblks);
    inode.flag.reset();

    store_hd();
  }

private:
  template <auto func>
  static inline void xfer(const inode_t *inode, uint8_t *buf, size_t buf_sz,
                          size_t offset = 0)
  {
    assert(inode->max_sz() >= buf_sz, TERM);
    const auto nblks = inode->nblks;

    uint8_t fst_blk = offset / BLK_SZ;
    size_t blk_offset = offset % BLK_SZ;
    size_t fst_blk_sz = min(buf_sz, BLK_SZ - blk_offset);

    uint8_t i = fst_blk;

    func(paddr(inode->blks[i++]) + blk_offset, buf, fst_blk_sz);
    buf += fst_blk_sz;
    buf_sz -= fst_blk_sz;

    while (buf_sz > 0 && i < nblks) {
      auto blk_sz = min(buf_sz, BLK_SZ);
      func(paddr(inode->blks[i++]), buf, blk_sz);

      buf += blk_sz;
      buf_sz -= blk_sz;
    }
  }

public:
  static void load(const inode_t *inode, uint8_t *buf, size_t buf_sz,
                   size_t off = 0)
  {
    xfer<desc.read>(inode, buf, buf_sz, off);
  }

  static void store(const inode_t *inode, const uint8_t *buf, size_t buf_sz,
                    size_t off = 0)
  {
    xfer<desc.write>(inode, (uint8_t *)buf, buf_sz, off);
    inode->end = max(inode->end, off + buf_sz);
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

