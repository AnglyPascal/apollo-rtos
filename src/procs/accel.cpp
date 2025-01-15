#include "nvm.h"
#include "sched.h"
#include "waitlist.h"

namespace procs
{

void *accel(void *param)
{
  nvm_t nvm{256};
  uint32_t *addr = (uint32_t *)*nvm;

  int n = 0;
  while (1) {
    n++;
    for (int i = 0; i < 256; i++) {
      addr[i] = n;
    }
    nvm.store();
    sched::sleep(1000);
  }

  return param;
}

} // namespace procs
