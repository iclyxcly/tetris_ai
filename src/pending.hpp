#pragma once
#include "const.h"
#include <deque>
#include "board.hpp"
namespace moenew
{
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
		Random rng;
		void rngify()
		{
			rng = Random(rng.next());
		}
		void push(const int amt, const int delay)
		{
			if (amt == 0)
			{
				return;
			}
			lines.emplace_back(amt, delay);
		}
		void decay()
		{
			if (lines.empty())
			{
				return;
			}
			for (auto &line : lines)
			{
				line.delay--;
			}
		}
		void decay_rem()
		{
			decay();
			while (!lines.empty() && lines[0].delay <= 0)
			{
				lines.pop_front();
			}
		}
		int estimate() const
		{
			int ret = 0;
			for (const auto &line : lines)
			{
				if (line.delay <= 0)
				{
					ret += line.amt;
				}
			}
			return ret;
		}
		double estimate_mess() const
		{
			double ret = 0;
			int cnt = 0;
			for (const auto &line : lines)
			{
				if (line.delay <= 0)
				{
					ret += line.amt;
					cnt++;
				}
			}
			return cnt == 0 ? 0 : ret / cnt;
		}
		int total()
		{
			if (lines.empty())
			{
				return 0;
			}
			return std::accumulate(lines.begin(), lines.end(), 0, [](int acc, PendingLine &line)
								   { return acc + line.amt; });
		}
		void cancel(int &amt)
		{
			while (amt > 0 && !lines.empty())
			{
				if (lines[0].amt <= amt)
				{
					amt -= lines[0].amt;
					lines.pop_front();
				}
				else
				{
					lines[0].amt -= amt;
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
		bool empty() const
		{
			return lines.empty();
		}
		int size() const
		{
			return lines.size();
		}
		Pending() : rng(rand()) {}
		Pending(Random &rng) : rng(rng) {}
		Pending(uint64_t seed) : rng(seed) {}
	};
}