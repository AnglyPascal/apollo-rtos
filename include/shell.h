#pragma once

#include "char_buffer.h"
#include "sched.h"
#include "types.h"

namespace shell
{

constexpr size_t args_len = 64;
using buffer = char_buffer<args_len>;

void init();

proc_def_t *match_cmd(string cmd);

} // namespace shell
