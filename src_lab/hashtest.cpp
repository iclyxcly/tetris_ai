#include "mino.hpp"
#include "minotemplate.h"
#include <unordered_map>
#include <cassert>
#include <bit>
#include <bitset>
#include <iostream>
using namespace moenew;
uint32_t coord_hashify(const Minos::Active &mino)
{
    return (mino.x + 2) + 34 * ((mino.y + 2) + 32 * mino.r);
}
uint64_t state_hashify(const Minos::Active &mino, const uint32_t data[4])
{
    uint64_t hash = 0;
    for (int i = 0; i < 4; ++i)
    {
        hash = hash * 7 + data[i];
    }
    return hash * 31 + mino.y;
}
bool check(const uint32_t data[4], const uint32_t data2[4])
{
    for (int i = 0; i < 4; ++i)
    {
        if (data[i] != data2[i])
        {
            return false;
        }
    }
    return true;
}

void test_state()
{
    for (int i = 0; i < 7; ++i)
    {
        std::unordered_map<uint64_t, bool> visited;
        std::unordered_map<uint64_t, Minos::Active> active_data;
        uint64_t max = 0;
        Minos::Active mino;
        for (int l = 0; l < 4; ++l)
        {
            for (int j = down_offset[i][l]; j < 30; ++j)
            {
                for (int k = left_offset[i][l]; k < 29 - right_offset[i][l]; ++k)
                {
                    // Collision support
                    mino.y = j - down_offset[i][l];
                    mino.x = k - left_offset[i][l];
                    mino.r = l;
                    uint64_t hash = state_hashify(mino, cache_get(i, l, k));
                    if (hash > max)
                    {
                        max = hash;
                    }
                    if (visited.find(hash) != visited.end() && !check(cache_get(i, l, k), cache_get(i, active_data[hash].r, active_data[hash].x + left_offset[i][active_data[hash].r])))
                    {
                        printf("Mino %c\n", type_to_char(i));
                        printf("collision: %lld\n", hash);
                        printf("x: %d, y: %d, r: %d\n", mino.x, mino.y, mino.r);
                        auto &collide_mino = active_data[hash];
                        printf("collides with: x: %d, y: %d, r: %d\n", collide_mino.x, collide_mino.y, collide_mino.r);
                        std::cout << std::bitset<32>(cache_get(i, l, k)[0]) << " (" + std::to_string(cache_get(i, l, k)[0]) << ")" << std::endl;
                        std::cout << std::bitset<32>(cache_get(i, l, k)[1]) << " (" + std::to_string(cache_get(i, l, k)[1]) << ")" << std::endl;
                        std::cout << std::bitset<32>(cache_get(i, l, k)[2]) << " (" + std::to_string(cache_get(i, l, k)[2]) << ")" << std::endl;
                        std::cout << std::bitset<32>(cache_get(i, l, k)[3]) << " (" + std::to_string(cache_get(i, l, k)[3]) << ")" << std::endl;
                        printf("Collides with:\n");
                        auto cache = cache_get(i, collide_mino.r, collide_mino.x + left_offset[i][collide_mino.r]);
                        std::cout << std::bitset<32>(cache[0]) << " (" + std::to_string(cache[0]) << ")" << std::endl;
                        std::cout << std::bitset<32>(cache[1]) << " (" + std::to_string(cache[1]) << ")" << std::endl;
                        std::cout << std::bitset<32>(cache[2]) << " (" + std::to_string(cache[2]) << ")" << std::endl;
                        std::cout << std::bitset<32>(cache[3]) << " (" + std::to_string(cache[3]) << ")" << std::endl;
                        printf("total find: %d/%d\n", visited.size(), (30 - down_offset[i][l]) * (29 - right_offset[i][l] - left_offset[i][l]) * 4);
                        exit(0);
                    }
                    visited[hash] = true;
                    active_data[hash] = mino;
                }
            }
        }
        printf("Mino %c\n", type_to_char(i));
        printf("hashmax: %lld\n", max);
    }
}
void test_coord()
{
    std::unordered_map<uint32_t, bool> visited;
    uint32_t max = 0;
    for (int i = -2; i < 30; ++i)
    {
        Minos::Active mino;
        mino.y = i;
        for (int j = -2; j < 32; ++j)
        {
            mino.x = j;
            for (int k = 0; k < 4; ++k)
            {
                mino.r = k;
                uint32_t hash = coord_hashify(mino);
                if (hash > max)
                {
                    max = hash;
                }
                if (visited.find(hash) != visited.end())
                {
                    printf("Coord\n");
                    printf("collision: %lld\n", hash);
                    printf("total find: %d/%d\n", visited.size(), 32 * 34 * 4);
                    exit(0);
                }
                visited[hash] = true;
            }
        }
    }
    printf("Coord\n");
    printf("hashmax: %lld\n", max);
}
int main()
{
    test_state();
    test_coord();
    return 0;
}