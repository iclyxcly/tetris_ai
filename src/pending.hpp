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
		std::string lines;
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
			char data;
			char line = amt;
			if (amt > 126)
			{
				line = 126;
			}
			data = line << 1;
			data |= delay > 0;
			lines += data;
		}
		void decay()
		{
			for (auto &line : lines)
			{
				if (line & 1)
				{
					line--;
				}
			}
		}
		int estimate() const
		{
			int ret = 0;
			for (const auto &line : lines)
			{
				if (!(line & 1))
				{
					ret += line >> 1;
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
				if (!(line & 1))
				{
					ret += line >> 1;
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
			return std::accumulate(lines.begin(), lines.end(), 0, [](int acc, char line)
								   { return acc + (line >> 1); });
		}
		void cancel(int &amt)
		{
			while (amt > 0 && !lines.empty())
			{
				if (lines[0] >> 1 > amt)
				{
					lines[0] -= amt << 1;
					amt = 0;
				}
				else
				{
					amt -= lines[0] >> 1;
					lines.erase(lines.begin());
				}
			}
		}
		void accept(Board &src, const double &mess)
		{
			while (!lines.empty() && !(lines[0] & 1))
			{
				int index = rng.next() % src.w;
				int acc = 0;
				for (int i = 0; i < lines[0] >> 1; i++)
				{
					++acc;
					if (rng.next() % 100 < mess * 100)
					{
						index = rng.next() % src.w;
					}
					src.rise(1, index);
				}
				lines.erase(lines.begin());
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