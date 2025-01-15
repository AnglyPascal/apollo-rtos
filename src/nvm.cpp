#include "nvm.h"
#include "flash.h"
#include "memory.h"

__extern_C__
uint8_t __nvm_start[],
    __nvm_end[];

void *nvm_t::operator*()
{
  return ram_addr;
}

nvm_t::nvm_t(uint32_t *addr, size_t len) : nvm_addr(addr), len(len)
{
  ram_addr = (uint32_t *)heap::malloc(len);
}

nvm_t::nvm_t(size_t len) : len(len)
{
  nvm_addr = (uint32_t *)__nvm_start;
  ram_addr = (uint32_t *)heap::malloc(len * sizeof(uint32_t));
}

nvm_t::~nvm_t()
{
  heap::free(ram_addr);
}

void nvm_t::store()
{
  flash::erase(nvm_addr);
  flash::write(nvm_addr, ram_addr, len);
}

void nvm_t::load()
{
  for (size_t i = 0; i < len; i++) {
    ram_addr[i] = nvm_addr[i];
    flash::wait();
  }
}

