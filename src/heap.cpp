#include "allocator.h"
#include "memory.h"
#include "serial.h"

namespace heap
{

namespace
{
allocator pool;
}

void *malloc(size_t sz)
{
  auto ptr = pool.alloc(sz);
  printf("malloc: %x\n", ptr);
  return ptr;
}

void free(void *ptr)
{
  printf("free: %x\n", ptr);
  pool.dealloc((uint8_t *)ptr);
}

void trace()
{
  printf("heap allocator: \r\n");
  pool.trace();
}

} // namespace heap

void *operator new(size_t sz)
{
  return heap::malloc(sz);
}

void operator delete(void *ptr)
{
  return heap::free(ptr);
}
