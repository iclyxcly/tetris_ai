#pragma once
#include "const.h"
#include "utils.hpp"
#include <deque>
#include <algorithm>
namespace moenew
{
	class FakeNext
	{
		public:
		std::string next;
		FakeNext()
		{
			fill();
		}
		void reset()
		{
			next.clear();
		}
		void fill(int max = 28)
		{
			if (next.size() >= max)
				return;
			std::string mix = {S, L, Z, I, T, O, J};
			static std::mt19937 rng(std::random_device{}());
			while (next.size() < max)
			{
				std::shuffle(mix.begin(), mix.end(), rng);
				next += mix;
			}
		}
		void pop()
		{
			next.erase(next.begin());
			fill();
		}
	};
	class Next
	{
	public:
		Next() : hold(X) {}
		std::string next;
		char hold;
		bool held;
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
			std::string mix = {S, L, Z, I, T, O, J};
			static std::mt19937 rng(std::random_device{}());
			while (next.size() < max)
			{
				std::shuffle(mix.begin(), mix.end(), rng);
				next += mix;
			}
		}
		void fill(FakeNext &fake_next)
		{
			for (int i = 0; i < fake_next.next.size() && next.size() < 15; ++i)
			{
				next.push_back(fake_next.next[i]);
			}
		}
		std::string to_string(int length = 5)
		{
			std::string temp;
			for (int i = 0; i < length; ++i)
			{
				temp += type_to_char(next[i]);
			}
			return temp;
		}
		bool swap()
		{
			if (held || next.size() <= 1 || hold == next[0])
				return false;
			held = true;
			if (hold == X)
			{
				hold = next.front();
				next.erase(next.begin());
				return true;
			}
			else
			{
				std::swap(hold, next[0]);
				return true;
			}
		}
		char pop()
		{
			char temp = next.front();
			next.erase(next.begin());
			held = false;
			return temp;
		}
		char peek() const
		{
			return next.front();
		}
		void push(char &src)
		{
			next.push_back(src);
		}
		void push(std::string &src)
		{
			next += src;
		}
		std::string get(int n)
		{
			return next.substr(0, n);
		}
	};
}