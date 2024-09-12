#include "movegen.hpp"
#include "minotemplate.h"
#include "board.hpp"
#include <iostream>
int main()
{
    using namespace moenew;
    Board board(10, 40);
    Minos::Active mino;
    mino.x = 3;
    mino.y = 17;
    mino.r = 0;
    Piece type = S;
    MoveGen movegen(board, mino, type);
    movegen.start();
    printf("coords: %lu\n", movegen.coords.size());
    printf("states: %lu\n", movegen.states.size());
    for (auto &i : movegen.result)
    {
        printf("x: %d, y: %d, r: %d\n", i.x, i.y, i.r);
        auto board_copy = board;
        auto data = cache_get(type, i.r, i.x);
        board_copy.paste(data, i.y, -down_offset[type][i.r], 4 + up_offset[type][i.r]);
        std::cout << board_copy.print(4);
    }
    return 0;
}