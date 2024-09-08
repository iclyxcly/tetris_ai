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
	std::string Board::print(int top) const
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
	void Board::set(int& x, int& y)
	{
		board[y] ^= X_INDEX[x];
		tidy();
	}
	bool Board::get(int& x, int& y) const
	{
		return board[y] & X_INDEX[x];
	}
	bool Board::line(int& y) const
	{
		static int width = w - 1;
		static auto& req = LINE_CLEAR[width];
		return board[y] & req;
	}
	void Board::clear()
	{
		std::memset(board, 0, sizeof(board));
		y_max = 0;
		cnt = 0;
	}
	int Board::flush()
	{
		int clear = 0;
		const auto& height_m1 = h - 1;
		for (int y = height_m1; y >= 0; y--)
		{
			if (line(y))
			{
				std::memmove(&board[y], &board[y + 1], sizeof(uint64_t) * (h - y));
				board[height_m1] = 0;
				clear++;
			}
		}
		tidy();
		return clear;
	}
	void Board::tidy()
	{
		y_max = 0;
		cnt = 0;
		for (int y = h - 1; y >= 0; y--)
		{
			if (board[y])
			{
				if (y_max == 0)
					y_max = y;
				cnt += std::popcount(board[y]);
			}
		}
	}
	void Board::rise(int& amt, int& i)
	{
		memmove(board + amt, board, sizeof(board) - amt * sizeof(uint64_t));
		memset(board, 0, amt * sizeof(uint64_t));
		const uint64_t line = LINE_CLEAR[w - 1] ^ ((uint64_t)1 << i);
		for (uint8_t i = 0; i < amt; i++)
		{
			board[i] = line;
		}
	}
}