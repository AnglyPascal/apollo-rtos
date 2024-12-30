#pragma once

#include "types.h"
#include <cstdint>

namespace timer
{

void init();

time_t now();

/* extern "C" uint32_t *timer1_swap(uint32_t *); */
/* extern "C" uint32_t *timer1_body(); */

} // namespace timer
