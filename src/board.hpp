#pragma once
#include <string>
#include <format>
#include <cstring>
#include <bit>
#include "mino.hpp"
#include "const.h"
#include "minotemplate.h"
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
			count = 0;
			std::memset(field, 0, sizeof(field));
		}
		Board(int w, int h)
		{
			this->w = w;
			this->h = h;
			y_max = 0;
			count = 0;
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
			return (field[y] & loc_c.of(w - 1)) == loc_c.of(w - 1);
		}
		void clear()
		{
			std::memset(field, 0, sizeof(field));
			y_max = 0;
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
			count -= clear * w;
			return clear;
		}
		void tidy()
		{
			int y = h;
			while (y > 0 && field[y - 1] == 0)
			{
				y--;
			}
			y_max = y;
			count = 0;
			while(--y >= 0)
			{
				count += __builtin_popcount(field[y]);
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
		void paste(const uint32_t data[4], const int y)
		{
			for (int i = 0; i < 4; i++)
			{
				if (y + i < 0)
				{
					continue;
				}
				field[y + i] |= data[i];
			}
			tidy();
		}
		bool integrate(const uint32_t data[4], const int &y) const
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
		uint8_t safe(int offset) const
		{
			uint8_t safe = DEFAULT_Y - offset; 
			while (safe > 0 && !get(DEFAULT_X, safe - 1) && !get(DEFAULT_X + 1, safe - 1) && !get(DEFAULT_X + 2, safe - 1) && !get(DEFAULT_X + 3, safe - 1))
			{
				--safe;
			}
			return (DEFAULT_Y - offset) - safe;
		}
		uint8_t safe() const
		{
			uint8_t safe = DEFAULT_Y; 
			while (safe > 0 && !get(DEFAULT_X, safe - 1) && !get(DEFAULT_X + 1, safe - 1) && !get(DEFAULT_X + 2, safe - 1) && !get(DEFAULT_X + 3, safe - 1))
			{
				--safe;
			}
			return DEFAULT_Y - safe;
		}
		uint8_t get_safe(std::string &next) const
		{
			if (!next.empty())
			{
				return safe(down_offset[next[0]][0]);
			}
			return safe();
		}
		double get_safe_rate(std::string &next) const
		{
			return (double)get_safe(next) / (next.empty() ? DEFAULT_Y : DEFAULT_Y - down_offset[next[0]][0]);
		}
		uint8_t y_max : 5;
		uint8_t w : 5;
		uint8_t h : 5;
		uint16_t count;
		uint16_t field[BOARD_HEIGHT];
	};
}