#include "core/sched.h"
#include "core/test.h"
#include "utility/debug.h"

TEST(test1) { return false; }

TEST(test2) { return true; }

namespace sched
{
void setup_procs(void) {}
void setup_startups(void) {}
} // namespace sched
