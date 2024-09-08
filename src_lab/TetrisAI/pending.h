#pragma once
#include "const.h"
#include <deque>
#include "board.h"
namespace moenew
{
	class Pending
	{
	public:
		struct PendingLine
		{
			int amt;
			int delay;
			PendingLine(int amt, int delay) : amt(amt), delay(delay) {}
		};
		std::deque<PendingLine> lines;
		random rng;
		void push(int amt, int delay);
		void decay();
		int estimate();
		int total();
		void cancel(int& amt);
		void accept(Board& src, double& mess);
	};
}