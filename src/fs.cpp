#include "fs.h"
#include "debug.h"
#include "hardware.h"
#include "timer.h"

__extern_C__
uint8_t __nvm_start[],
    __nvm_end[];

namespace
{

// can use about 220 nvms of flash memory
constexpr size_t NFILES = 110;

constexpr fd_t free_head_fd = NFILES;
constexpr fd_t files_head_fd = NFILES + 1;
constexpr fd_t null_fd = _max<fd_t>;

alignas(uint32_t) struct {
  inode_t inodes[NFILES + 2];
  bool is_valid = false;
  // other information about the fs;
} tbl;
static_assert(sizeof(tbl) <= pg_sz);

uint32_t *tbl_addr = (uint32_t *)__nvm_end;
nvm_t *tbl_pg;

} // namespace

constexpr uint32_t *off_to_pg_addr(size_t off)
{
  return (uint32_t *)(__nvm_start + off * pg_sz);
}

file_t::file_t(fd_t fd, bool write_en)
    : inode{tbl.inodes + fd}, addr{heap::malloc(inode->sz)},
      pg1{off_to_pg_addr(2 * fd), (uint32_t *)addr, inode->pg1_sz()},
      pg2{off_to_pg_addr(2 * fd + 1), (uint32_t *)((uint8_t *)addr + pg_sz),
          inode->pg2_sz()},
      write_en{write_en}
{
  debug<INFO>("%d, %x, %x, %d\r\n", fd, addr, off_to_pg_addr(2 * fd),
              tbl.inodes[fd].sz);
  load();
}

void file_t::load() const
{
  auto sz = inode->sz;
  pg1.load();
  if (sz > pg_sz) {
    pg2.load();
  }
}

void file_t::erase() const
{
  if (!write_en)
    return;

  auto sz = inode->sz;
  pg1.erase();
  if (sz > pg_sz) {
    pg2.erase();
  }
}

void file_t::store() const
{
  if (!write_en)
    return;

  auto sz = inode->sz;
  pg1.store();
  if (sz > pg_sz) {
    pg2.store();
  }
}

file_t::~file_t()
{
  if (write_en && inode->write_en) {
    inode->write_en = false;
  }
  heap::free(addr);
}

void *file_t::operator*()
{
  return addr;
}

/////////////////////
// Filesystem impl //
/////////////////////

namespace fs
{

inline void init()
{
  for (fd_t fd = 0; fd < NFILES; fd++) {
    tbl.inodes[fd] = {
        fd, fd_t(fd + 1), fd_t(fd - 1), false, 0,
    };
  }

  tbl.inodes[0].prev = free_head_fd;
  tbl.inodes[NFILES - 1].next = null_fd;

  tbl.inodes[free_head_fd] = {free_head_fd, 0, null_fd, true};
  tbl.inodes[files_head_fd] = {files_head_fd, null_fd, null_fd, true};

  tbl.is_valid = true;
}

void mount()
{
  tbl_pg = new nvm_t{(uint32_t *)tbl_addr, (uint32_t *)&tbl, sizeof(tbl)};
  tbl_pg->load();
  if (!tbl.is_valid)
    init();
}

void store()
{
  tbl_pg->store();
}

inline void extract(inode_t &inode)
{
  auto prev = inode.prev, next = inode.next;
  tbl.inodes[prev].next = next;
  if (next != null_fd) {
    tbl.inodes[next].prev = prev;
  }
}

inline void insert(inode_t &head, inode_t &inode)
{
  inode.next = head.next;
  inode.prev = head.fd;

  tbl.inodes[head.next].prev = inode.fd;
  head.next = inode.fd;
}

fd_t create()
{
  auto fd = tbl.inodes[free_head_fd].next;
  auto &inode = tbl.inodes[fd];

  extract(inode);
  insert(tbl.inodes[files_head_fd], inode);

  return fd;
}

void remove(fd_t fd)
{
  auto &inode = tbl.inodes[fd];

  extract(inode);
  insert(tbl.inodes[free_head_fd], inode);
}

file_t *open(fd_t fd, size_t sz, uint32_t flags)
{
  assert(sz > 0 && fd >= 0 && fd < NFILES);

  auto &inode = tbl.inodes[fd];

  bool write_en = flags & O_WRITE;
  if (inode.write_en && write_en) {
    debug<ERROR>("file %d is already open for write\r\n", fd);
    return nullptr;
  }

  bool new_file = false;
  if (inode.sz == 0) {
    if ((flags & O_CREATE) == 0) {
      debug<ERROR>("file %d doesn't exist\r\n", fd);
      return nullptr;
    }

    new_file = true;
    inode.sz = sz;
  }

  auto file = new file_t{fd, write_en};
  if (new_file) {
    file->erase();
  }

  inode.write_en = write_en;
  return file;
}

void close(file_t *file)
{
  delete file;
}

void fstat(fd_t fd) {}

} // namespace fs

extern "C" {
void *__dso_handle = nullptr, *_fini = nullptr;
}
