#pragma once

#include "memory.h"
#include "nvm.h"
#include "types.h"

using fn_t = uint8_t;

struct inode_t {
  fn_t fn;
  fn_t next;
  fn_t prev;

  uint8_t in_use;
  size_t sz;
};

class file_t;

#define O_CREATE 0x00000001
#define O_WRITE 0x00000002

namespace fs
{

void mount();
void store();

file_t open(fn_t fn, size_t sz, uint32_t flags);

void trace();

} // namespace fs

class fd_t;

class file_t
{
public:
  fn_t fn;

private:
  fd_t *fd;
  bool w_en;

  friend file_t fs::open(fn_t fn, size_t sz, uint32_t flags);

public:
  file_t(fn_t fn, fd_t *fd, bool w_en);

  void load();
  void store();
  void erase();
  void *operator*();
  bool is_valid() const;

  ~file_t();
};

