#pragma once
#include "const.h"
#include <string>
#include "mino.h"
namespace moenew
{
	class Board
	{
	public:
		Board();
		Board(int w, int h);
		std::string print(int top) const;
		void set(int& x, int& y);
		bool get(int& x, int& y) const;
		bool line(int& y) const;
		void clear();
		int flush();
		void tidy();
		void rise(int& amt, int& i);
		int y_max;
		int cnt;
		int w;
		int h;
		uint64_t board[BOARD_HEIGHT];
	};
}