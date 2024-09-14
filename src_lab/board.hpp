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
			return (board[y] & req) == req;
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
			for (int y = y_max; y >= 0; y--)
			{
				if (line(y))
				{
					clear++;
					trim(y);
				}
			}
			tidy();
			return clear;
		}
		void tidy()
		{
			int y = h - 1;
			while (y > 0 && board[y] == 0)
			{
				y--;
			}
			y_max = y;
			cnt = 0;
			for (; y >= 0; y--)
			{
				if (board[y] != 0)
				{
					cnt += std::popcount(loc_c.of(w) & board[y]);
				}
			}
		}
		void rise(int amt, int i)
		{
			auto data = loc_c.of(w - 1) & ~loc_x.of(i);
			for (int y = y_max; y >= 0; y--)
			{
				board[y + amt] = board[y];
			}
			for (int y = 0; y < amt; y++)
			{
				board[y] = data;
			}
			tidy();
		}
		void trim(int &y)
		{
			for (int i = y; i < y_max && i < h - 1; i++)
			{
				board[i] = board[i + 1];
			}
			board[y_max] = 0;
			--y_max;
		}
		void paste(const uint32_t data[4], const int y, const int start, const int end)
		{
			for (int i = start; i < end; i++)
			{
				board[y + i] |= data[i];
			}
		}
		bool integrate(const uint32_t data[4], const int &y)
		{
			for (int i = 0; i < 4; i++)
			{
				if (y + i < 0)
				{
					continue;
				}
				if (board[i + y] & data[i])
				{
					return false;
				}
			}
			return true;
		}
		int8_t y_max;
		int cnt;
		int w;
		int h;
		uint64_t board[BOARD_HEIGHT];
	};
}