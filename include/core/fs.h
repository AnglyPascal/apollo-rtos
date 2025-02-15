#pragma once

#include "core/file.h"
#include "drivers/flash.h"

namespace flash
{
void mount();
void format();

file_t open(fn_t fn, size_t sz, uint32_t flags);

void *mmap(file_t &file, size_t sz);

template <typename T>
T *mmap(file_t &file)
{
  return (T *)mmap(file, sizeof(T));
}

void mmap(file_t &file, uint8_t *addr, size_t sz);

template <typename T>
void mmap(file_t &file, T &obj)
{
  mmap(file, (uint8_t *)&obj, sizeof(T));
}

void unmap(file_t &file);

void load(file_t &file);
void store(file_t &file);

} // namespace flash

namespace fs
{
void init();
}
