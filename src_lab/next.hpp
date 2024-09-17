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
		bool held;
		void init()
		{
			hold = X;
			fill();
		}
		void reset()
		{
			next.clear();
			hold = X;
			held = false;
		}
		void fill(int max = 28)
		{
			if (next.size() >= max)
				return;
			std::deque<Piece> mix = {S, L, Z, I, T, O, J};
			static std::mt19937 rng(std::random_device{}());
			while (next.size() < max)
			{
				std::shuffle(mix.begin(), mix.end(), rng);
				next.insert(next.end(), mix.begin(), mix.end());
			}
		}
		bool swap()
		{
			if (held || next.size() == 0 || hold == next[0])
				return false;
			held = true;
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
			held = false;
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
		std::deque<Piece> get(int n)
		{
			std::deque<Piece> temp;
			for (int i = 0; i < n; ++i)
			{
				temp.push_back(next[i]);
			}
			return temp;
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