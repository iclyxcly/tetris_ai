#pragma once
#include "const.h"
#include <string>
namespace moenew
{
    class Board
    {
    public:
        Board();
        Board(int w, int h);
        std::string print(int top);
        void set(int &x, int &y);
        bool get(int &x, int &y);
        // bool isFull();
        // int clear();
        void tidy();
        int y_max;
        int cnt;
        int w;
        int h;
        uint64_t board[BOARD_HEIGHT];
    };
}