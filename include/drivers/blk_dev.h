#pragma once

#include "core/types.h"

template <typename _blk_addr_t, size_t _blk_sz>
class blk_dev
{
public:
  static constexpr size_t blk_sz = _blk_sz;
  using addr_t = _blk_addr_t;

public:
};
