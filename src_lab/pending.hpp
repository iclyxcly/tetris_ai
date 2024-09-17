#pragma once
#include "const.h"
#include <deque>
#include "board.hpp"
namespace moenew
{
	class Pending
	{
	private:
		struct Random
		{
		private:
			uint64_t state;
		public:
			Random(uint64_t seed) : state(seed) {}
			uint64_t next()
			{
				state ^= state >> 12;
				state ^= state << 25;
				state ^= state >> 27;
				return state * 2685821657736338717ull;
			}
		};
	public:
		struct PendingLine
		{
			int amt;
			int delay;
			PendingLine(int amt, int delay) : amt(amt), delay(delay) {}
		};
		std::deque<PendingLine> lines;
		Random rng;
		void push(const int amt, const int delay)
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
		void accept(Board &src, const double &mess)
		{
			while (!lines.empty() && lines[0].delay <= 0)
			{
				int index = rng.next() % src.w;
				int acc = 0;
				for (int i = 0; i < lines[0].amt; i++)
				{
					++acc;
					if (rng.next() % 100 < mess * 100)
					{
						index = rng.next() % src.w;
					}
					src.rise(1, index);
				}
				lines.pop_front();
			}
		}
		bool empty()
		{
			return lines.empty();
		}
		int size()
		{
			return lines.size();
		}
		Pending() : rng(rand()) {}
		Pending(Random &rng) : rng(rng) {}
		Pending(uint64_t seed) : rng(seed) {}
	};
}