#include "pending.h"
#include <numeric>
namespace moenew
{
	void Pending::push(int amt, int delay)
	{
		lines.emplace_back(amt, delay);
	}
	void Pending::decay()
	{
		for (auto& line : lines)
		{
			line.delay--;
		}
	}
	int Pending::estimate()
	{
		int ret = 0;
		for (auto& line : lines)
		{
			if (line.delay <= 0)
			{
				ret += line.amt;
			}
		}
		return ret;
	}
	int Pending::total()
	{
		return std::accumulate(lines.begin(), lines.end(), 0, [](int acc, PendingLine& line) { return acc + line.amt; });
	}
	void Pending::cancel(int& amt)
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
	void Pending::accept(Board& src, double& mess)
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
					src.rise(acc, index);
					acc = 0;
					index = cheese(rng);
				}
			}
			if (acc)
			{
				src.rise(acc, index);
			}
		}
	}
}