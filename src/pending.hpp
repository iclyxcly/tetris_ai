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
		int lines[2];
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
			lines[delay] += amt;
		}
		void decay()
		{
			lines[0] = lines[1];
			lines[1] = 0;
		}
		int estimate() const
		{
			return lines[0];
		}
		int total()
		{
			return lines[0] + lines[1];
		}
		void cancel(int &amt)
		{
			while (amt > 0 && (lines[0] > 0 || lines[1] > 0))
			{
				--amt;
				if (lines[0] == 0)
				{
					--lines[1];
				}
				else
				{
					--lines[0];
				}
			}
		}
		void accept(Board &src, const double &mess)
		{
			if (lines[0] > 0)
			{
				int index = rng.next() % src.w;
				src.rise_alloc(lines[0]);
				for (int i = 0; i < lines[0]; i++)
				{
					if (rng.next() % 100 < mess * 100)
					{
						index = rng.next() % src.w;
					}
					src.rise(i, index);
				}
				src.tidy();
			}
			decay();
		}
		bool empty() const
		{
			return lines[0] == 0 && lines[1] == 0;
		}
		void clear()
		{
			lines[0] = 0;
			lines[1] = 0;
		}
		Pending() : rng(rand()) {}
		Pending(Random &rng) : rng(rng) {}
		Pending(uint64_t seed) : rng(seed) {}
	};
}