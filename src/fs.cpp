#include "fs.h"
#include "debug.h"
#include "hardware.h"
#include "timer.h"

__extern_C__
uint8_t __nvm_start[],
    __nvm_end[];

// can use about 220 pages of flash memory
constexpr size_t NFILES = 110;

constexpr fn_t free_fn = NFILES;
constexpr fn_t files_fn = NFILES + 1;
constexpr fn_t null_fn = _max<fn_t>;

alignas(uint32_t) struct {
public:
  bool is_valid = false;
  // other information about the fs;

private:
  inode_t inodes[NFILES + 2];

public:
  inode_t &operator[](size_t idx)
  {
    return inodes[idx];
  }
} tbl;

static_assert(sizeof(tbl) <= pg_sz);
static_assert(sizeof(tbl) % sizeof(uint32_t) == 0);

uint32_t *tbl_addr = (uint32_t *)__nvm_end;
nvm_t tbl_pg;

////////////
/// fd_t ///
////////////

class fd_t
{
public:
  inode_t *inode = nullptr;
  void *addr = nullptr;

  nvm_t pg1;
  nvm_t pg2;

  uint8_t w_cnt = 0;
  uint8_t r_cnt = 0;

  friend class file_t;
  friend class fd_tbl_t;

  static inline uint32_t *off2pg(size_t off)
  {
    return (uint32_t *)((size_t)__nvm_start + off * pg_sz);
  }

private:
  fd_t() {}

  void open(inode_t *_inode)
  {
    auto fn = _inode->fn;
    debug<TRACE>("%d, %x, %x, %d\r\n", fn, addr, off2pg(2 * fn), tbl[fn].sz);

    inode = _inode;
    addr = heap::malloc(inode->sz);
    pg1 = {off2pg(2 * fn), (uint32_t *)addr, inode->pg1_sz()};
    if (inode->pg2_sz() > 0)
      pg2 = {off2pg(2 * fn + 1), (uint32_t *)((uint8_t *)addr + pg_sz),
             inode->pg2_sz()};
    w_cnt = 0;
    r_cnt = 0;

    load();
  }

  void acquire(bool w_en)
  {
    w_cnt += w_en;
    r_cnt++;
  }

  void release(bool w_en)
  {
    w_cnt -= w_en;
    r_cnt--;
    if (r_cnt == 0)
      close();
  }

  void close()
  {
    inode = nullptr;

    heap::free(addr);
    addr = nullptr;

    pg1 = {};
    pg2 = {};
  }

  void load() const
  {
    auto sz = inode->sz;
    pg1.load();
    if (sz > pg_sz) {
      pg2.load();
    }
  }

  void erase() const
  {
    auto sz = inode->sz;
    pg1.erase();
    if (sz > pg_sz) {
      pg2.erase();
    }
  }

  void store() const
  {
    auto sz = inode->sz;
    pg1.store();
    if (sz > pg_sz) {
      pg2.store();
    }
  }

  void *operator*()
  {
    return addr;
  }

public:
  void fstat()
  {
    auto fn = inode->fn;
    printf("\t%d. sz: %d, pg: %x, rt: %x\r\n", fn, inode->sz, off2pg(2 * fn),
           addr);
  }
};

//////////////
/// fd_tbl ///
//////////////

constexpr size_t N_OPEN_FILES = 16;
class fd_tbl_t
{
  fd_t fds[N_OPEN_FILES] = {};

public:
  fd_t *find(inode_t *inode)
  {
    size_t i = 0;
    fd_t *empty_fd = nullptr;

    while (i < N_OPEN_FILES) {
      fd_t *fd = fds + i;

      if (fd->inode == inode) {
        return fd;
      }

      if (fd->inode == nullptr) {
        empty_fd = fd;
      }

      i++;
    }

    return empty_fd;
  }

  fd_t *open(inode_t *inode)
  {
    auto fd = find(inode);
    if (fd->inode != inode)
      fd->open(inode);
    return fd;
  }

  void close(fd_t *fd)
  {
    fd->close();
  }

  void trace() const
  {
    for (auto fd : fds) {
      if (fd.inode != nullptr)
        fd.fstat();
    }
  }
} fd_tbl;

//////////////
/// file_t ///
//////////////

file_t::file_t(fn_t fn, fd_t *fd, bool w_en) : fn{fn}, fd{fd}, w_en{w_en}
{
  fd->acquire(w_en);
}

void file_t::load()
{
  fd->load();
}

void file_t::store()
{
  if (w_en)
    fd->store();
}

void file_t::erase()
{
  if (w_en)
    fd->erase();
}

void *file_t::operator*()
{
  return **fd;
}

file_t::~file_t()
{
  fd->release(w_en);
}

/////////////////////
// Filesystem impl //
/////////////////////

namespace fs
{

inline void init()
{
  for (fn_t fn = 0; fn < NFILES; fn++) {
    tbl[fn] = {
        fn, fn_t(fn + 1), fn_t(fn - 1), false, 0,
    };
  }

  tbl[0].prev = free_fn;
  tbl[NFILES - 1].next = null_fn;

  tbl[free_fn] = {free_fn, 0, null_fn, true};
  tbl[files_fn] = {files_fn, null_fn, null_fn, true};
}

void mount(bool is_valid)
{
  tbl_pg = {(uint32_t *)tbl_addr, (uint32_t *)&tbl, sizeof(tbl)};
  tbl_pg.load();
  if (!is_valid)
    init();
}

void store()
{
  tbl_pg.store();
}

inline void extract(inode_t &inode)
{
  auto prev = inode.prev, next = inode.next;
  tbl[prev].next = next;
  if (next != null_fn) {
    tbl[next].prev = prev;
  }
}

inline void insert(inode_t &head, inode_t &inode)
{
  inode.next = head.next;
  inode.prev = head.fn;

  tbl[head.next].prev = inode.fn;
  head.next = inode.fn;
}

fn_t create()
{
  auto fn = tbl[free_fn].next;
  auto &inode = tbl[fn];

  extract(inode);
  insert(tbl[files_fn], inode);

  return fn;
}

void remove(fn_t fn)
{
  auto &inode = tbl[fn];

  extract(inode);
  insert(tbl[free_fn], inode);
}

file_t open(fn_t fn, size_t sz, uint32_t flags)
{
  assert(sz > 0 && fn >= 0 && fn < NFILES);

  auto &inode = tbl[fn];

  if (!inode.in_use) {
    assert(flags & O_CREATE);

    inode.sz = sz;
    inode.in_use = true;

    fs::store();
  }

  bool w_en = flags & O_WRITE;
  return {fn, fd_tbl.open(&inode), w_en};
}

void fstat(fn_t fn)
{
  auto inode = &tbl[fn];
  auto fd = fd_tbl.find(inode);
  fd->fstat();
}

void trace()
{
  printf("filesystem:\r\n");
  fd_tbl.trace();
}

} // namespace fs

extern "C" {
void *__dso_handle = nullptr, *_fini = nullptr;
}
