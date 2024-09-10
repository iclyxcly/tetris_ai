#pragma once
#include "const.h"
#include <deque>
#include "board.hpp"
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
		void push(int amt, int delay)
		{
			lines.emplace_back(amt, delay);
		}
		void decay()
		{
			for (auto &line : lines)
			{
				line.delay--;
			}
		}
		int estimate()
		{
			int ret = 0;
			for (auto &line : lines)
			{
				if (line.delay <= 0)
				{
					ret += line.amt;
				}
			}
			return ret;
		}
		int total()
		{
			return std::accumulate(lines.begin(), lines.end(), 0, [](int acc, PendingLine &line)
								   { return acc + line.amt; });
		}
		void cancel(int &amt)
		{
			while (amt > 0 && !lines.empty())
			{
				if (lines.front().amt <= amt)
				{
					amt -= lines.front().amt;
					lines.pop_front();
				}
				else
				{
					lines.front().amt -= amt;
					amt = 0;
				}
			}
		}
		void accept(Board &src, double &mess)
		{
			static rng_range cheese(0, src.w - 1);
			static rng_range mess_(0, 99);
			while (!lines.empty() && lines[0].delay <= 0)
			{
				int index = cheese(rng);
				int acc = 0;
				for (int i = 0; i < lines[0].amt; i++)
				{
					++acc;
					if (mess_(rng) < mess * 100)
					{
						src.set(index, src.y_max + acc);
					}
				}
				lines.pop_front();
			}
		}
	};
}