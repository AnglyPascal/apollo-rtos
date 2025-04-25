#pragma once

#include "core/types.h"
#include "utility/bitset.h"
#include "utility/debug.h"

template <typename desc_t, desc_t desc>
struct _inode_t {
  using blk_addr_t = typename desc_t::blk_addr_t;
  using addr_t = typename desc_t::addr_t;

  blk_addr_t blks[desc.max_num_blks];
  mutable addr_t end;
  const uint8_t nblks;
  mutable uint8_t flag;

  _inode_t() : end{0}, nblks{0}, flag{0} {}

  size_t max_sz() const { return nblks * desc_t::blk_sz; }

  void reset()
  {
    { // not needed, but still
      const_cast<uint8_t &>(nblks) = 0;
      end = 0;
    }
    flag = 0;
  }

  bool in_use() const { return flag & IN_USE; }
  void set_use() { flag |= IN_USE; }

  file_type_t ft() const { return (flag & IS_CHAR) ? CHAR : BIN; }
  void set_ft(file_type_t _ft)
  {
    if (_ft == CHAR)
      flag |= IS_CHAR;
    else
      flag &= ~IS_CHAR;
  }

  bool is_perm() const { return flag & IS_PERM; }
  void set_perm() { flag |= IS_PERM; }

  // invariant: is_valid() <=> end != 0
  bool is_valid() const { return flag & IS_VALID; }
  bool set_valid() const { return flag |= IS_VALID; }

private:
  enum {
    IN_USE = 1 << 0,
    IS_CHAR = 1 << 1,
    IS_PERM = 1 << 2,
    IS_VALID = 1 << 3,
  };
};

template <typename desc_t, desc_t desc>
  requires(desc.start < desc.end) &&
          (desc.start + desc.hd_addr * desc_t::blk_sz + desc.hd_sz <= desc.end)
class _fs_t
{
public:
  using blk_addr_t = typename desc_t::blk_addr_t;
  using addr_t = typename desc_t::addr_t;
  using fn_t = typename desc_t::fn_t;
  using inode_t = _inode_t<desc_t, desc>;

  static constexpr auto BLK_SZ = desc_t::blk_sz;
  static constexpr uint32_t BLK_MAGIC = 0xbabebabe;
  static constexpr size_t N_BLKS = (desc.end - desc.start) / BLK_SZ;

  static_assert(N_BLKS <= (1 << 8));

  struct hd_t {
    inode_t tbl[desc.n_inodes];
    uint32_t magic;
    bitset<N_BLKS> free_set;
  };

  static_assert(sizeof(hd_t) <= desc.hd_sz);

  inline static hd_t hd;

private:
  static inline bool alloc_blks(inode_t &inode)
  {
    blk_addr_t addr = 0;
    for (uint8_t i = 0; i < inode.nblks; i++) {
      addr = hd.free_set.next(addr);
      if (addr == MAX<size_t>)
        return false;

      hd.free_set.erase(addr);
      inode.blks[i] = addr++;
    }
    return true;
  }

  static inline void dealloc_blks(inode_t &inode)
  {
    for (uint8_t i = 0; i < inode.nblks; i++)
      hd.free_set.insert(inode.blks[i]);
  }

  static constexpr addr_t paddr(blk_addr_t blk_addr)
  {
    auto addr = desc.start + (addr_t)blk_addr * BLK_SZ;
    assert(desc.start <= addr && addr < desc.end, H_RESET);
    return addr;
  }

  static inline void load_hd()
  {
    desc.read(paddr(desc.hd_addr), (uint8_t *)&hd, sizeof(hd));
  }

  static inline void store_hd()
  {
    desc.write(paddr(desc.hd_addr), (uint8_t *)&hd, sizeof(hd));
  }

  static inline void store_inode(const inode_t &inode, bool with_freeset)
  {
    const auto hd_addr = paddr(desc.hd_addr);

    const auto inode_off = (size_t)&inode - (size_t)&hd;
    desc.write(hd_addr + inode_off, (uint8_t *)&inode, sizeof(inode));

    if (with_freeset) {
      const auto freeset_off = (size_t)&hd.free_set - (size_t)&hd;
      desc.write(hd_addr + freeset_off, (uint8_t *)&hd.free_set,
                 sizeof(hd.free_set));
    }
  }

  static inline void update_hd(const inode_t &inode, bool with_freeset)
  {
    if constexpr (desc.is_ram)
      store_inode(inode, with_freeset);
    else
      store_hd();
  }

public:
  static inline bool valid() { return hd.magic == BLK_MAGIC; }

  static inline void format()
  {
    hd.magic = BLK_MAGIC;
    for (auto &inode : hd.tbl)
      inode.reset();
    hd.free_set.set_all();

    store_hd();
  }

  static inline void mount()
  {
    load_hd();
    if (!valid())
      format();
  }

  static inline void umount() { store_hd(); }

  // return the corresponding inode, and whether the file was just opened
  static inline pair<const inode_t *, bool> open(fn_t fn, size_t sz,
                                                 uint32_t flags)
  {
    assert(fn >= 0 && fn < desc.n_inodes, TERM);

    auto &inode = hd.tbl[fn];
    const bool in_use = inode.in_use();
    const bool to_create = flags & O_CREATE;

    if (in_use || !to_create) {
      assert(in_use, TERM, "file does not exist\r\n");
      if (sz != 0)
        debug<TRACE>("passing size (%d) to open existing file %d\r\n", sz, fn);
      return {&inode, false};
    }

    assert(to_create, TERM, "not creating non-existent file %d\r\n", fn);
    inode.set_use();

    uint8_t nblks = roundup(max(sz, BLK_SZ), BLK_SZ) / BLK_SZ;
    assert(nblks <= desc.max_num_blks, TERM);
    const_cast<uint8_t &>(inode.nblks) = nblks;

    auto success = alloc_blks(inode);
    assert(success, TERM, "block allocation failed for file %d\r\n", fn);

    if constexpr (desc.is_ram)
      inode.set_ft(flags & O_CHAR_FILE ? CHAR : BIN);
    else
      inode.set_ft(BIN);

    if (flags & O_PERM)
      inode.set_perm();

    inode.end = 0;

    update_hd(inode, true);

    return {&inode, true};
  }

  static inline void close(const inode_t *inode) { update_hd(*inode, false); }

  static inline bool remove(fn_t fn, bool forced = false)
  {
    auto &inode = hd.tbl[fn];

    if (!inode.in_use()) {
      debug<WARN>("deleting non-existent file does nothing %d\r\n", fn);
      return false;
    }

    if (!forced && inode.is_perm()) {
      debug<ERROR>("cannot delete permanent file %d\r\n", fn);
      return false;
    }

    dealloc_blks(inode);
    inode.reset();

    update_hd(inode, true);

    return true;
  }

private:
  // return the number of bytes written/read
  template <auto func>
  static inline size_t xfer(const inode_t *inode, uint8_t *buf,
                            const size_t buf_sz, size_t offset = 0)
  {
    assert(inode->max_sz() >= offset + buf_sz, TERM);
    const auto nblks = inode->nblks;

    uint8_t fst_blk = offset / BLK_SZ;
    size_t blk_offset = offset % BLK_SZ;
    size_t fst_blk_sz = min(buf_sz, BLK_SZ - blk_offset);

    uint8_t i = fst_blk;

    auto rem = buf_sz;
    func(paddr(inode->blks[i++]) + blk_offset, buf, fst_blk_sz);
    buf += fst_blk_sz;
    rem -= fst_blk_sz;

    while (rem > 0 && i < nblks) {
      auto blk_sz = min(rem, BLK_SZ);
      func(paddr(inode->blks[i++]), buf, blk_sz);

      buf += blk_sz;
      rem -= blk_sz;
    }

    // FIXME: return the actual number of written bytes
    return buf_sz - rem;
  }

public:
  static size_t load(const inode_t *inode, void *buf, size_t buf_sz,
                     size_t off = 0)
  {
    return xfer<desc.read>(inode, (uint8_t *)buf, buf_sz, off);
  }

  static size_t store(const inode_t *inode, const void *buf, size_t buf_sz,
                      size_t off = 0)
  {
    auto nbytes = xfer<desc.write>(inode, (uint8_t *)buf, buf_sz, off);

    bool prev_invalid = inode->end == 0;
    inode->end = max(inode->end, off + nbytes);

    if (inode->end != 0) {
      inode->set_valid();
      if (prev_invalid)
        update_hd(*inode, false);
    }

    return nbytes;
  }

  static inline void trace()
  {
    auto free_blks = hd.free_set.size();
    auto free_sz = (size_t)free_blks * BLK_SZ;
    auto used_sz = N_BLKS * BLK_SZ - free_sz;

    debug<INFO>("  |  "                              //
                "free_blks: " BLUE "%d" DEFAULT ", " //
                "free: " BLUE "%d" DEFAULT ", "      //
                "in use: " BLUE "%d" DEFAULT "\r\n",
                free_blks, free_sz, used_sz);
  }
};

