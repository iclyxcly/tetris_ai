#pragma once
#include <string>
#include <format>
#include <cstring>
#include <bit>
#include "mino.hpp"
#include "const.h"
namespace moenew
{
	class Board
	{
	public:
		Board()
		{
			w = BOARD_WIDTH;
			h = BOARD_HEIGHT;
			y_max = 0;
			cnt = 0;
			std::memset(board, 0, sizeof(board));
		}
		Board(int w, int h)
		{
			this->w = w;
			this->h = h;
			y_max = 0;
			cnt = 0;
			std::memset(board, 0, sizeof(board));
		}
		std::string print(int top) const
		{
			std::string ret;
			for (int y = top; y >= 0; y--)
			{
				ret += std::format("{:2d}|", y);
				for (int x = 0; x < w; x++)
				{
					ret += get(x, y) ? "[]" : "  ";
				}
				ret += "|\n";
			}
			return ret;
		}
		void set(int x, int y)
		{
			board[y] ^= loc_x.of(x);
			tidy();
		}
		bool get(int x, int y) const
		{
			return board[y] & loc_x.of(x);
		}
		bool line(int y) const
		{
			static int width = w - 1;
			static auto &req = loc_c.of(width);
			return board[y] & req;
		}
		void clear()
		{
			std::memset(board, 0, sizeof(board));
			y_max = 0;
			cnt = 0;
		}
		int flush()
		{
			int clear = 0;
			for (int y = 0; y < y_max; y++)
			{
				if (line(y))
				{
					rise(clear, y);
				}
			}
			return clear;
		}
		void tidy()
		{
			int y = h - 1;
			while (y >= 0 && board[y] == 0)
			{
				y--;
			}
			y_max = y;
		}
		void rise(int &amt, int &i)
		{
			for (int y = i; y < y_max; y++)
			{
				board[y] = board[y + 1];
			}
			board[y_max] = 0;
			amt++;
			tidy();
		}
		void paste(const uint32_t data[], const int y, const int start, const int end)
		{
			for (int i = start; i < end; i++)
			{
				board[y + i] |= data[i];
			}
		}
		int y_max;
		int cnt;
		int w;
		int h;
		uint64_t board[BOARD_HEIGHT];
	};
}