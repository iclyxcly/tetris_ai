#pragma once
#include "board.hpp"
#include "mino.hpp"
#include <queue>
#include <vector>
namespace moenew
{
	class MoveGen
	{
	public:
		Board &target;
		Minos::Active &cur;
		std::queue<Minos::Active> search;
		std::vector<bool> visited;
		std::vector<bool> result;
		uint16_t hashify(const Minos::Active &mino) const
		{
			return mino.x + mino.y * target.w + mino.r * 4;
		}
		MoveGen(Board &target, Minos::Active &init) : target(target), cur(init)
		{
			visited.resize(4352);
			result.resize(1024);
			search.push(init);
		}
	};
}