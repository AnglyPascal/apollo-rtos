#pragma once

#include "core/file.h"
#include "drivers/flash.h"

namespace flash
{
void mount();
void format();

file_t open(fn_t fn, size_t sz, uint32_t flags);

void *mmap(file_t &file, size_t sz);
void unmap(file_t &file);

template <typename T>
T *mmap(file_t &file)
{
  return (T *)mmap(file, sizeof(T));
}

void load(file_t &file);
void store(file_t &file);

} // namespace flash

