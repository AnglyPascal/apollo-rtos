#pragma once

#include "types.h"

class allocator
{
  struct chunk_t {
    chunk_t *next = nullptr;
    size_t sz = 0;
    uint8_t block[];
  };

  chunk_t head;

  static constexpr size_t chunk_header_sz = sizeof(chunk_t *) + sizeof(size_t);

public:
  allocator() = default;

  uint8_t *alloc(size_t sz);
  void dealloc(uint8_t *ptr);

  void trace();
};
