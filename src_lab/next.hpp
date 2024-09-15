#pragma once
#include "const.h"
#include "utils.hpp"
#include <deque>
#include <algorithm>
namespace moenew
{
	class Next
	{
	public:
		Next() : hold(X) {}
		std::deque<Piece> next;
		Piece hold;
		void init()
		{
			hold = X;
			fill();
		}
		void fill(int max = 28)
		{
			if (next.size() >= max)
				return;
			std::deque<Piece> mix = {S, L, Z, I, T, O, J};
			static random rng;
			while (next.size() < max)
			{
				std::shuffle(mix.begin(), mix.end(), rng);
				next.insert(next.end(), mix.begin(), mix.end());
			}
		}
		bool swap()
		{
			if (next.size() == 0 || hold == next[0])
				return false;
			if (hold == X)
			{
				hold = next.front();
				next.pop_front();
				return true;
			}
			else
			{
				std::swap(hold, next[0]);
				return true;
			}
		}
		Piece pop()
		{
			Piece temp = next.front();
			next.pop_front();
			return temp;
		}
		Piece peek() const
		{
			return next.front();
		}
		void push(Piece &src)
		{
			next.push_back(src);
		}
		void push(std::deque<Piece> &src)
		{
			next.insert(next.end(), src.begin(), src.end());
		}
		std::string join()
		{
			std::string temp;
			for (auto &i : next)
			{
				temp += type_to_char(i);
			}
			return temp;
		}
	};
}