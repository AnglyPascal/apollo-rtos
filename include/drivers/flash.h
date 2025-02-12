#pragma once

#include "core/file.h"
#include "core/types.h"
#include "drivers/fs_desc.h"

namespace flash
{
using addr_t = uint32_t;

using fn_t = uint8_t;
using blk_addr_t = uint8_t;
using addr_t = uint32_t;
using nblks_t = uint8_t;

void write(addr_t addr, uint8_t *buf, size_t buf_sz);
void read(addr_t addr, uint8_t *buf, size_t buf_sz);

static constexpr size_t PAGE_SZ = 1 << 10;
static constexpr addr_t FLASH_END = 256 * PAGE_SZ;
static constexpr size_t N_PAGES = 180;

static constexpr size_t blk_sz = PAGE_SZ;
using desc_t = fs_desc_t<fn_t, blk_addr_t, addr_t, nblks_t, blk_sz>;

inline constexpr desc_t desc = {
    .write = flash::write,
    .read = flash::read,

    .start = FLASH_END - N_PAGES * PAGE_SZ,
    .end = FLASH_END,

    .fs_hd_addr = N_PAGES - 1,
    .fs_hd_sz = PAGE_SZ,

    .max_num_blks = 8,
    .n_inodes = 32,
};

using file_t = _file_t<desc_t>;

} // namespace flash
