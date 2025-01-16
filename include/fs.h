#pragma once

#include "memory.h"
#include "nvm.h"
#include "types.h"

using fd_t = uint8_t;

struct inode_t {
  fd_t fd;
  fd_t next;
  fd_t prev;
  uint8_t write_en;
  size_t sz;

  size_t pg1_sz() const
  {
    return min(sz, pg_sz);
  }

  size_t pg2_sz() const
  {
    return max(sz, pg_sz) - pg_sz;
  }
};

class file_t
{
  inode_t *const inode;
  void *const addr;

  nvm_t pg1;
  nvm_t pg2;

  const bool write_en;

public:
  file_t(fd_t fd, bool write_en);

  void *operator*();
  void store() const;
  void load() const;
  void erase() const;

  ~file_t();
};

#define O_CREATE 0x00000001
#define O_WRITE 0x00000002

namespace fs
{

void mount();
void store();
file_t *open(fd_t fd, size_t sz, uint32_t flags);
void close(file_t *file);

} // namespace fs
