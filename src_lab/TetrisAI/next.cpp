#include "next.h"
namespace moenew
{
	Next::Next() : hold(X) {}

	void Next::init()
	{
		hold = X;
		fill();
	}

	void Next::fill(int max)
	{
		if (next.size() >= max)
			return;
		std::deque<Piece> mix = { S, L, Z, I, T, O, J };
		static random rng;
		while (next.size() < max)
		{
			std::shuffle(mix.begin(), mix.end(), rng);
			next.insert(next.end(), mix.begin(), mix.end());
		}
	}

	bool Next::swap()
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

	Piece Next::pop()
	{
		Piece temp = next.front();
		next.pop_front();
		fill();
		return temp;
	}

	void Next::push(Piece& src)
	{
		next.push_back(src);
	}

	void Next::push(std::deque<Piece>& src)
	{
		next.insert(next.end(), src.begin(), src.end());
	}
}