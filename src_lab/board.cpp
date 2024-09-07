#include "board.h"
#include <format>
#include <cstring>
#include <bit>
namespace moenew
{
    Board::Board()
    {
        w = BOARD_WIDTH;
        h = BOARD_HEIGHT;
        y_max = 0;
        cnt = 0;
        std::memset(board, 0, sizeof(board));
    }
    Board::Board(int w, int h)
    {
        this->w = w;
        this->h = h;
        y_max = 0;
        cnt = 0;
        std::memset(board, 0, sizeof(board));
    }
    std::string Board::print(int top)
    {
        std::string ret;
        for (int y = top; y >= 0; y--)
        {
            ret += std::format("{:2d}|", y);
            for (int x = 0; x < w; x++)
            {
                ret += get(x, y) ? "[]" : "  ";
            }
            ret += "\n";
        }
        return ret;
    }
    void Board::set(int &x, int &y)
    {
        board[y] ^= X_INDEX[x];
        tidy();
    }
    bool Board::get(int &x, int &y)
    {
        return board[y] & X_INDEX[x];
    }
    void Board::tidy()
    {
        y_max = 0;
        cnt = 0;
        for (int y = 0; y < h; y++)
        {
            if (board[y])
            {
                y_max = y;
                cnt += std::popcount(board[y]);
            }
        }
    }
}