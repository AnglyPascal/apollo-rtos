#pragma once

#include "fs/fs.h"
#include "utility/format.h"

namespace curr_proc
{
fn_t out_fn();
} // namespace curr_proc

template <typename... Args>
void fprintf(fn_t fn, Args... args)
{
  if (fn == stdout)
    return do_printf(serial::os, std::forward<Args>(args)...);

  ofstream os{fn, O_CREATE};
  return os.printf(std::forward<Args>(args)...);
}

template <typename... Args>
void printf(Args... args)
{
  fprintf(curr_proc::out_fn(), std::forward<Args>(args)...);
}

