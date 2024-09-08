#pragma once
#include "const.h"
#include <deque>
namespace moenew
{
	class Next
	{
	public:
		Next();
		std::deque<Piece> next;
		Piece hold;
		void init();
		void fill(int max = 28);
		bool swap();
		Piece pop();
		void push(Piece& src);
		void push(std::deque<Piece>& src);
	};
}