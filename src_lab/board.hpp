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
			std::memset(field, 0, sizeof(field));
		}
		Board(int w, int h)
		{
			this->w = w;
			this->h = h;
			y_max = 0;
			cnt = 0;
			std::memset(field, 0, sizeof(field));
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
			field[y] ^= loc_x.of(x);
			tidy();
		}
		bool get(int x, int y) const
		{
			return field[y] & loc_x.of(x);
		}
		bool line(int y) const
		{
			static int width = w - 1;
			static auto &req = loc_c.of(width);
			return (field[y] & req) == req;
		}
		void clear()
		{
			std::memset(field, 0, sizeof(field));
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
			while (y > 0 && field[y] == 0)
			{
				y--;
			}
			y_max = y;
			cnt = 0;
			for (; y >= 0; y--)
			{
				if (field[y] != 0)
				{
					cnt += std::popcount(loc_c.of(w) & field[y]);
				}
			}
		}
		void rise(int amt, int i)
		{
			auto data = loc_c.of(w - 1) & ~loc_x.of(i);
			for (int y = y_max; y >= 0; y--)
			{
				field[y + amt] = field[y];
			}
			for (int y = 0; y < amt; y++)
			{
				field[y] = data;
			}
			tidy();
		}
		void trim(int &y)
		{
			for (int i = y; i < y_max && i < h - 1; i++)
			{
				field[i] = field[i + 1];
			}
			field[y_max] = 0;
			--y_max;
		}
		void paste(const uint32_t data[4], const int y, const int start, const int end)
		{
			for (int i = start; i < end; i++)
			{
				field[y + i] |= data[i];
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
				if (field[i + y] & data[i])
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
		uint64_t field[BOARD_HEIGHT];
	};
}