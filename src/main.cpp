#include <cstdint>
#include <bitset>
#include <iostream>
int main()
{
    uint32_t test = 0b0011;
    std::cout << std::bitset<32>(test << 28);
    return 0;
}