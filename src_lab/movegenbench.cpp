#include "movegen.hpp"
#include "minotemplate.h"
#include "board.hpp"
#include <chrono>
#include <iostream>
int main()
{
    using namespace moenew;
    Board board(10, 40);
    board.set(0, 15);
    board.tidy();
    Minos::Active mino;
    mino.x = 3;
    mino.y = 17;
    mino.r = 0;
    Piece type = S;
    auto now = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i)
    {
        MoveGen movegen(board, mino, type);
        movegen.start();
    }
    auto end = std::chrono::high_resolution_clock::now();
    MoveGen movegen(board, mino, type);
    movegen.start();
    for (auto &i : movegen.result)
    {
        printf("x: %d, y: %d, r: %d\n", i.x, i.y, i.r);
        auto board_copy = board;
        auto data = cache_get(type, i.r, i.x);
        board_copy.paste(data, i.y, -down_offset[type][i.r], 4 + up_offset[type][i.r]);
        std::cout << board_copy.print(4);
    }
    printf("coords: %lu\n", movegen.coords.size());
    printf("landpoints: %lu\n", movegen.landpoints.size());
    printf("time: %ld ns\n", std::chrono::duration_cast<std::chrono::nanoseconds>(end - now).count());
    return 0;
}