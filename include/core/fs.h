#pragma once

#include "core/file.h"
#include "core/fs_impl.h"
#include "drivers/fs_desc.h"

namespace _flash
{
using addr_t = uint32_t;
using fn_t = uint8_t;
using blk_addr_t = uint8_t;
using nblks_t = uint8_t;

void write(addr_t addr, uint8_t *buf, size_t buf_sz);
void read(addr_t addr, uint8_t *buf, size_t buf_sz);

static constexpr size_t BLK_SZ = 1 << 10;
static constexpr addr_t FLASH_END = 256 * BLK_SZ;
static constexpr size_t N_BLKS = 180;

using desc_t = fs_desc_t<fn_t, blk_addr_t, addr_t, nblks_t, BLK_SZ>;

static constexpr size_t N_FS_BLKS = 1;
inline constexpr desc_t desc = {
    .write = write,
    .read = read,

    .start = FLASH_END - N_BLKS * BLK_SZ,
    .end = FLASH_END,

    .fs_hd_addr = N_BLKS - N_FS_BLKS,
    .fs_hd_sz = BLK_SZ * N_FS_BLKS,

    .max_num_blks = 8,
    .n_inodes = 32,
};

inline file_type_t ft_func(uint32_t flag) { return BIN; }

} // namespace _flash

namespace _fram
{
using addr_t = uint16_t;
using fn_t = uint8_t;
using blk_addr_t = uint8_t;
using nblks_t = uint8_t;

void write(addr_t addr, uint8_t *buf, size_t buf_sz);
void read(addr_t addr, uint8_t *buf, size_t buf_sz);

static constexpr addr_t FRAM_END = 1 << 15;
static constexpr size_t BLK_SZ = 1 << 7;
static constexpr size_t N_BLKS = FRAM_END / BLK_SZ;

using desc_t = fs_desc_t<fn_t, blk_addr_t, addr_t, nblks_t, BLK_SZ>;

static constexpr size_t N_FS_BLKS = 8;
inline constexpr desc_t desc = {
    .write = write,
    .read = read,

    .start = 0x0,
    .end = FRAM_END,

    .fs_hd_addr = N_BLKS - N_FS_BLKS,
    .fs_hd_sz = BLK_SZ * N_FS_BLKS,

    .max_num_blks = 32,
    .n_inodes = 32,
};

using file_t = _file_t<desc_t>;

inline file_type_t ft_func(uint32_t flag) { return CHAR; }

} // namespace _fram

struct flash
    : public fs_impl_t<_flash::desc_t, _flash::desc, 8, _flash::ft_func> {
  using file_t = _file_t<_flash::desc_t>;
};

struct fram : public fs_impl_t<_fram::desc_t, _fram::desc, 16, _fram::ft_func> {
  using file_t = _file_t<_fram::desc_t>;
};

namespace fs
{
inline volatile bool _first_boot = false;
inline bool first_boot() { return _first_boot; }

inline void init()
{
  flash::mount();
  fram::mount();

  _first_boot = flash::first_boot();
}

inline void trace()
{
  debug<INFO>("flash fs: \r\n");
  flash::trace();
  debug<INFO>("fram fs: \r\n");
  fram::trace();
}
} // namespace fs

