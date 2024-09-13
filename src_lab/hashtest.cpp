#include "mino.hpp"
#include "minotemplate.h"
#include <unordered_map>
#include <cassert>
#include <bit>
#include <bitset>
#include <iostream>
using namespace moenew;
uint32_t coord_hashify(const MoveData &mino)
{
    return (mino.get_x() + 2) + 34 * ((mino.get_y() + 2) + 32 * mino.get_r());
}
uint64_t state_hashify(const MoveData &mino, const uint32_t data[4])
{
    uint64_t hash = data[0];
    hash = hash * 31 + data[1];
    hash = hash * 31 + data[2];
    hash = hash * 31 + data[3];
    return hash * 31 + mino.get_y();
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
        std::unordered_map<uint64_t, MoveData> active_data;
        uint64_t max = 0;
        MoveData mino;
        for (int l = 0; l < 4; ++l)
        {
            for (int j = down_offset[i][l]; j < 30; ++j)
            {
                for (int k = left_offset[i][l]; k < 29 - right_offset[i][l]; ++k)
                {
                    // Collision support
                    mino.set_y(j - down_offset[i][l]);
                    mino.set_x(k - left_offset[i][l]);
                    mino.set_r(l);
                    uint64_t hash = state_hashify(mino, cache_get(i, l, k));
                    if (hash > max)
                    {
                        max = hash;
                    }
                    if (visited.find(hash) != visited.end() && !check(cache_get(i, l, k), cache_get(i, active_data[hash].get_r(), active_data[hash].get_x() + left_offset[i][active_data[hash].get_r()])))
                    {
                        printf("Mino %c\n", type_to_char(i));
                        printf("collision: %lu\n", hash);
                        printf("x: %d, y: %d, r: %d\n", mino.get_x(), mino.get_y(), mino.get_r());
                        auto &collide_mino = active_data[hash];
                        printf("collides with: x: %d, y: %d, r: %d\n", collide_mino.get_x(), collide_mino.get_y(), collide_mino.get_r());
                        std::cout << std::bitset<32>(cache_get(i, l, k)[0]) << " (" + std::to_string(cache_get(i, l, k)[0]) << ")" << std::endl;
                        std::cout << std::bitset<32>(cache_get(i, l, k)[1]) << " (" + std::to_string(cache_get(i, l, k)[1]) << ")" << std::endl;
                        std::cout << std::bitset<32>(cache_get(i, l, k)[2]) << " (" + std::to_string(cache_get(i, l, k)[2]) << ")" << std::endl;
                        std::cout << std::bitset<32>(cache_get(i, l, k)[3]) << " (" + std::to_string(cache_get(i, l, k)[3]) << ")" << std::endl;
                        printf("Collides with:\n");
                        auto cache = cache_get(i, collide_mino.get_r(), collide_mino.get_x() + left_offset[i][collide_mino.get_r()]);
                        std::cout << std::bitset<32>(cache[0]) << " (" + std::to_string(cache[0]) << ")" << std::endl;
                        std::cout << std::bitset<32>(cache[1]) << " (" + std::to_string(cache[1]) << ")" << std::endl;
                        std::cout << std::bitset<32>(cache[2]) << " (" + std::to_string(cache[2]) << ")" << std::endl;
                        std::cout << std::bitset<32>(cache[3]) << " (" + std::to_string(cache[3]) << ")" << std::endl;
                        printf("total find: %zu/%d\n", visited.size(), (30 - down_offset[i][l]) * (29 - right_offset[i][l] - left_offset[i][l]) * 4);
                        exit(0);
                    }
                    visited[hash] = true;
                    active_data[hash] = mino;
                }
            }
        }
        printf("Mino %c\n", type_to_char(i));
        printf("hashmax: %lu\n", max);
    }
}
void test_coord()
{
    std::unordered_map<int16_t, bool> visited;
    int16_t max = 0;
    for (int i = -2; i < 30; ++i)
    {
        MoveData mino;
        mino.set_y(i);
        for (int j = -2; j < 32; ++j)
        {
            mino.set_x(j);
            for (int k = 0; k < 4; ++k)
            {
                mino.set_r(k);
                int16_t hash = mino.hash();
                if (hash > max)
                {
                    max = hash;
                }
                if (visited.find(hash) != visited.end())
                {
                    printf("Coord\n");
                    printf("collision: %u\n", hash);
                    printf("total find: %zd/%d\n", visited.size(), 32 * 34 * 4);
                    exit(0);
                }
                visited[hash] = true;
            }
        }
    }
    printf("Coord\n");
    printf("hashmax: %u\n", max);
}
int main()
{
    test_state();
    test_coord();
    return 0;
}