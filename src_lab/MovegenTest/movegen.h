#pragma once
#include "../TetrisAI/board.h"
#include <queue>
#include <vector>
namespace moenew
{
	class MoveGen
	{
	public:
		Board& target;
		Minos::Active& cur;
		Minos::Active& cur;
		std::queue<Minos::Active> search;
		std::vector<bool> visited, result;
		MoveGen(Board& target, Minos::Active &init);
	};
}