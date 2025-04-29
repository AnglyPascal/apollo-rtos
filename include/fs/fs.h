#pragma once

#include "fs/fs_desc.h"
#include "fs/fs_impl.h"

#include "fs/ifstream.h"
#include "fs/ofstream.h"

namespace _flash
{
using addr_t = uint32_t;
using fn_t = uint8_t;
using blk_addr_t = uint8_t;

size_t write(addr_t addr, uint8_t *buf, size_t buf_sz);
size_t read(addr_t addr, uint8_t *buf, size_t buf_sz);

static constexpr size_t BLK_SZ = 1 << 10;
static constexpr addr_t FLASH_END = 256 * BLK_SZ;
static constexpr size_t N_BLKS = 128;

using desc_t = fs_desc_t<fn_t, blk_addr_t, addr_t, BLK_SZ>;

static constexpr size_t N_FS_BLKS = 1;
inline constexpr desc_t desc = {
    .write = write,
    .read = read,

    .write_sync = write,
    .read_sync = read,

    .start = FLASH_END - N_BLKS * BLK_SZ,
    .end = FLASH_END,

    .hd_addr = N_BLKS - N_FS_BLKS,
    .hd_sz = BLK_SZ * N_FS_BLKS,

    .max_num_blks = 8,
    .n_inodes = 16,

    .n_open_files = 8,

    .is_ram = false,
};

inline bool first_boot()
{
  static constexpr uint32_t FIRST_BOOT_MAGIC = 0xbebebabe;
  using hd_t = _fs_t<desc_t, desc>::hd_t;
  size_t addr = desc.start + desc.hd_addr * BLK_SZ + offsetof(hd_t, magic);

  uint32_t magic;
  read(addr, (uint8_t *)&magic, sizeof(magic));
  bool fst = magic != FIRST_BOOT_MAGIC;

  if (fst) {
    magic = FIRST_BOOT_MAGIC;
    write(addr, (uint8_t *)&magic, sizeof(magic));
  }

  return fst;
}
} // namespace _flash

namespace _fram
{
using addr_t = uint16_t;
using fn_t = uint8_t;
using blk_addr_t = uint8_t;

size_t write(addr_t addr, uint8_t *buf, size_t buf_sz);
size_t read(addr_t addr, uint8_t *buf, size_t buf_sz);

size_t write_sync(addr_t addr, uint8_t *buf, size_t buf_sz);
size_t read_sync(addr_t addr, uint8_t *buf, size_t buf_sz);

static constexpr addr_t FRAM_END = 1 << 15;
static constexpr size_t BLK_SZ = 1 << 7;
static constexpr size_t N_BLKS = FRAM_END / BLK_SZ;

using desc_t = fs_desc_t<fn_t, blk_addr_t, addr_t, BLK_SZ>;

static constexpr size_t N_FS_BLKS = 8;
inline constexpr desc_t desc = {
    .write = write,
    .read = read,

    .write_sync = write_sync,
    .read_sync = read_sync,

    .start = 0x0,
    .end = FRAM_END,

    .hd_addr = N_BLKS - N_FS_BLKS,
    .hd_sz = BLK_SZ * N_FS_BLKS,

    .max_num_blks = 24,
    .n_inodes = 32,

    .n_open_files = 16,

    .is_ram = true,
};
} // namespace _fram

#if FLASH_FS == 1
using flash = fs_impl_t<_flash::desc>;
#endif

using fram = fs_impl_t<_fram::desc>;

using ifstream = _ifstream<_fram::desc>;
using ofstream = _ofstream<_fram::desc>;


namespace fs
{
inline volatile bool _first_boot = false;
inline bool first_boot() { return _first_boot; }

inline void init()
{
#if FLASH_FS == 1
  flash::mount();
#endif
  fram::mount();

#if FLASH_FS == 1
  _first_boot = !flash::valid();
#else
  _first_boot = _flash::first_boot();
#endif
}

inline void flush()
{
#if FLASH_FS == 1
  flash::umount();
#endif
  fram::umount();
}

inline void trace(fn_t fn = null_fn)
{
#if FLASH_FS == 1
  if (fn == null_fn)
    debug<INFO>(BOLD "flash fs: \r\n" DEFAULT);
  flash::trace(fn);
#endif

  if (fn == null_fn)
    debug<INFO>(BOLD "fram fs: \r\n" DEFAULT);
  fram::trace(fn);
}
} // namespace fs

