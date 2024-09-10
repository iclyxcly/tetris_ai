#include "const.h"
#include <bitset>
#include <iostream>
int main()
{
    using namespace moenew;
    for (int i = 0; i < 64; ++i)
    {
        std::bitset<64> b(loc_x.of(i));
        std::cout << b << std::endl;
    }
    std::cout << std::endl;
    for (int i = 0; i < 64; ++i)
    {
        std::bitset<64> b(loc_c.of(i));
        std::cout << b << std::endl;
    }
}