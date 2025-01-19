#include "recover.h"

static recover_t recover __attribute__((section(".recover")))
__attribute__((__used__)) = {};

inline constexpr uint32_t magic_value = 0xdeadbeef;

bool is_reset()
{
  return recover.magic == magic_value;
}

void set_magic()
{
  recover.magic = magic_value;
}
