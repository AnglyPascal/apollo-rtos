#include "utility/bitset.h"
#include "core/test.h"
#include "utility/debug.h"

BEGIN_SUITE(bitset)

TEST(basic_operations)
{
  bool result = true;

  {
    bitset<32> bs;
    result &= bs.empty() && bs.begin() == bs.end();
  }

  {
    bitset<64> bs;
    bool inserted = bs.insert(2);
    result &= inserted && bs.contains(2) && bs.size() == 1 && !bs.insert(2);
  }

  {
    bitset<64> bs;
    bs.insert(3);
    bool erased = bs.erase(3);
    result &= erased && !bs.contains(3) && bs.size() == 0 && !bs.erase(3);
  }

  {
    bitset<8> bs;
    bs.insert(0); // First bit in byte
    bs.insert(7); // Last bit in byte
    result &= bs.contains(0) && bs.contains(7) && bs.size() == 2;
  }

  {
    bitset<8> bs;
    bs.set_all();

    bool all_set = true;
    for (size_t i = 0; i < 8; i++)
      all_set &= bs.contains(i);

    result &= all_set && bs.size() == 8 && bs.erase(3) && bs.size() == 7;
  }

  {
    bitset<128> bs;
    bool first = bs.insert(0);
    bool last = bs.insert(127);
    result &= first && last && bs.erase(0) && bs.erase(127) && bs.empty();
  }

  {
    bitset<1> bs;
    bool first_insert = bs.insert(0);
    bool second_insert = bs.insert(0);
    result &= first_insert && !second_insert && bs.erase(0) && bs.empty();
  }

  {
    bitset<16> bs;
    bs.insert(1);
    bs.insert(3);
    bs.insert(5);

    result &= bs.size() == 3;

    bs.erase(3);
    result &= bs.size() == 2 && bs.erase(5) && bs.size() == 1;
  }

  return result;
}

TEST(iteration)
{
  bool result = true;

  {
    bitset<16> bs;
    bs.insert(3);
    bs.insert(7);
    bs.insert(11);
    result &= bs.next(0) == 3 && bs.next(4) == 7 && bs.next(8) == 11;
  }

  {
    bitset<16> bs;
    bs.insert(2);
    bs.insert(5);
    bs.insert(9);

    uint32_t nums[] = {2, 5, 9};
    size_t i = 0;
    for (auto idx : bs)
      result &= (nums[i++] == idx);
  }

  return result;
}

END_SUITE()
