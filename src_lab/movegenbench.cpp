#include "movegen.hpp"
#include "minotemplate.h"
#include "board.hpp"
#include <chrono>
#include <iostream>
int main()
{
    srand(time(nullptr));
    using namespace moenew;
    Board board(10, 40);
    // for (int i = 2; i < 21; i += 3)
    // {
    //     board.set(2, i);
    //     board.set(5, i);
    //     board.set(8, i);
    // }
    
    // for (int i = 0; i < 9; i ++)
    // {
    //     board.set(0, i);
    //     board.set(9, i);
    // }
    // for (int i = 0; i < 5; i ++)
    // {
    //     board.set(1, i);
    //     board.set(8, i);
    // }
    // for (int i = 0; i < 2; i ++)
    // {
    //     board.set(2, i);
    //     board.set(7, i);
    // }
    //     board.set(3, 0);
    //     board.set(6, 0);
    board.tidy();
    MoveData mino;
    mino.set_x(3);
    mino.set_y(22);
    mino.set_r(0);
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    int runs = 0;
    int total = 0;
    do
    {
        MoveGen movegen(board, mino, static_cast<Piece>(rand() % 7));
        movegen.start();
        ++runs;
        total += movegen.landpoints.size();
    } while (duration_cast<milliseconds>(high_resolution_clock::now() - now).count() < 1000);
    auto end = high_resolution_clock::now();
    Piece type = static_cast<Piece>(rand() % 7);
    MoveGen movegen(board, mino, type);
    movegen.start();
    for (auto &i : movegen.result)
    {
        printf("x: %d, y: %d, r: %d\n", i.get_x(), i.get_y(), i.get_r());
        auto board_copy = board;
        auto data = cache_get(type, i.get_r(), i.get_x());
        board_copy.paste(data, i.get_y());
        std::cout << board_copy.print(10);
    }
    printf("coords: %lu\n", movegen.coords.size());
    printf("nodes: %lu\n", movegen.result.size());
    printf("cps: %d\n", runs);
    printf("nps: %d\n", total);
    printf("per call: %f ns\n", duration_cast<nanoseconds>(end - now).count() / static_cast<double>(runs));
    printf("per node: %f ns\n", duration_cast<nanoseconds>(end - now).count() / static_cast<double>(total));
    return 0;
}