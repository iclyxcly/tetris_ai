#pragma once
#include "board.hpp"
#include "mino.hpp"
#include "minotemplate.h"
#include <queue>
#include <unordered_set>
#include <vector>
namespace moenew
{
	std::atomic<std::size_t> max_coords(0);
	std::atomic<std::size_t> max_states(0);
	class MoveGen
	{
	private:
		enum Operation
		{
			Down = 0,
			Left = 1,
			Right = 2,
			CW = 3,
			CCW = 4,
			_180 = 5
		};
		struct Expansion : public Minos::Active
		{
			int avoid;
			Expansion(Minos::Active &data, int avoid) : Minos::Active(data), avoid(avoid) {}
		};

	public:
		Board &target;
		const Minocache *data;
		const int *up;
		const int *down;
		const int *left;
		const int *right;
		Piece &type;
		std::queue<Expansion> search;
		std::vector<Minos::Active> result;
		std::unordered_set<uint16_t> coords;
		std::unordered_set<uint64_t> states;
		constexpr uint16_t hashify(const Minos::Active &mino) const
		{
			return (mino.x + 2) + 34 * ((mino.y + 2) + 32 * mino.r);
		}
		constexpr uint64_t state_hashify(const Minos::Active &mino, const uint32_t data[4])
		{
			return data[0] * 7 + data[1] * 7 + data[2] * 7 + data[3] * 7 + mino.y;
		}
		bool integrate(const int8_t &x, const int8_t &y, const int8_t &r) const
		{
			const auto *rows = data->get(r, x);
			for (int i = -down[r], j = 4 + up[r]; i < j; ++i)
			{
				if (target.board[y + i] & rows[i])
				{
					return false;
				}
			}
			return true;
		}
		bool test_up(const Minos::Active &mino) const
		{
			return integrate(mino.x, mino.y + 1, mino.r);
		}
		bool test_down(const Minos::Active &mino) const
		{
			int y = mino.y - 1;
			return y >= down[mino.r] && integrate(mino.x, y, mino.r);
		}
		bool test_left(const Minos::Active &mino) const
		{
			int x = mino.x - 1;
			return x >= left[mino.r] && integrate(x, mino.y, mino.r);
		}
		bool test_right(const Minos::Active &mino) const
		{
			int x = mino.x + 1;
			return x <= target.w - right[mino.r] - 4 && integrate(x, mino.y, mino.r);
		}
		bool test_cw(const Minos::Active &mino) const
		{
			int rightmost = target.w - right[mino.r] - 3;
			if (within(mino.x, left[mino.r], rightmost) && mino.y >= down[mino.r])
			{
				if (integrate(mino.x, mino.y, mino.r))
				{
					return true;
				}
			}
			const auto *offset = clockwise(type, mino.r);
			const auto &max = clockwise_size(type, mino.r);
			for (int i = 0; i < max; i++)
			{
				int x = mino.x + offset[i].x;
				int y = mino.y + offset[i].y;
				if (within(x, left[mino.r], rightmost) && y >= down[mino.r])
				{
					if (integrate(x, y, mino.r))
					{
						return true;
					}
				}
			}
			return false;
		}
		bool test_ccw(const Minos::Active &mino) const
		{
			int rightmost = target.w - right[mino.r] - 3;
			if (within(mino.x, left[mino.r], rightmost) && mino.y >= down[mino.r])
			{
				if (integrate(mino.x, mino.y, mino.r))
				{
					return true;
				}
			}
			const auto *offset = counterclockwise(type, mino.r);
			const auto &max = counter_clockwise_size(type, mino.r);
			for (int i = 0; i < max; i++)
			{
				int x = mino.x + offset[i].x;
				int y = mino.y + offset[i].y;
				if (within(x, left[mino.r], rightmost) && y >= down[mino.r])
				{
					if (integrate(x, y, mino.r))
					{
						return true;
					}
				}
			}
			return false;
		}
		bool test_180(const Minos::Active &mino) const
		{
			int rightmost = target.w - right[mino.r] - 3;
			if (within(mino.x, left[mino.r], rightmost) && mino.y >= down[mino.r])
			{
				if (integrate(mino.x, mino.y, mino.r))
				{
					return true;
				}
			}
			const auto *offset = _180_spin(type, mino.r);
			const auto &max = _180_spin_size(type, mino.r);
			for (int i = 0; i < max; i++)
			{
				int x = mino.x + offset[i].x;
				int y = mino.y + offset[i].y;
				if (within(x, left[mino.r], rightmost) && y >= down[mino.r])
				{
					if (integrate(x, y, mino.r))
					{
						return true;
					}
				}
			}
			return false;
		}
		bool allspin(const Minos::Active &mino) const
		{
			return mino.last_rotate && !test_up(mino) && !test_down(mino) && !test_left(mino) && !test_right(mino);
		}
		bool immobile(const Minos::Active &mino) const
		{
			return !test_up(mino) && !test_down(mino) && !test_left(mino) && !test_right(mino);
		}
		void expand(Expansion &node)
		{
			// todo next: fix cw, ccw, 180
			// todo next: expand left right down on each spin
			Minos::Active next = node;
			if (node.avoid != CW)
			{
				next.r = (next.r + 1) & 3;
				auto hash = hashify(next);
				if (coords.find(hash) == coords.end() && test_cw(next))
				{
					next.last_rotate = true;
					coords.insert(hash);
					search.emplace(next, CCW);
					next.r = (next.r - 1) & 3;
				}
				else
				{
					next.r = (next.r - 1) & 3;
				}
			}
			if (node.avoid != CCW)
			{
				next.r = (next.r - 1) & 3;
				auto hash = hashify(next);
				if (coords.find(hash) == coords.end() && test_ccw(next))
				{
					next.last_rotate = true;
					coords.insert(hash);
					search.emplace(next, CW);
					next.r = (next.r + 1) & 3;
				}
				else
				{
					next.r = (next.r + 1) & 3;
				}
			}
			if (node.avoid != _180)
			{
				next.r = (next.r + 2) & 3;
				auto hash = hashify(next);
				if (coords.find(hash) == coords.end() && test_180(next))
				{
					next.last_rotate = true;
					coords.insert(hash);
					search.emplace(next, _180);
					next.r = (next.r - 2) & 3;
				}
				else
				{
					next.r = (next.r - 2) & 3;
				}
			}
			if (node.avoid != Down)
			{
				next.y--;
				auto hash = hashify(next);
				if (coords.find(hash) == coords.end() && test_down(next))
				{
					next.last_rotate = false;
					coords.insert(hash);
					search.emplace(next, -1);
					next.y++;
				}
				else
				{
					next.y++;
				}
			}
			if (node.avoid != Left)
			{
				next.x--;
				auto hash = hashify(next);
				if (coords.find(hash) == coords.end() && test_left(next))
				{
					next.last_rotate = false;
					coords.insert(hash);
					search.emplace(next, Right);
					next.x++;
				}
				else
				{
					next.x++;
				}
			}
			if (node.avoid != Right)
			{
				next.x++;
				auto hash = hashify(next);
				if (coords.find(hash) == coords.end() && test_right(next))
				{
					next.last_rotate = false;
					coords.insert(hash);
					search.emplace(next, Left);
					next.x--;
				}
				else
				{
					next.x--;
				}
			}
		}
		void start()
		{
			while (!search.empty())
			{
				auto &node = search.front();
				expand(node);
				auto hash = state_hashify(node, data->get(node.r, node.x));
				if (states.find(hash) == states.end())
				{
					states.insert(hash);
					result.push_back(node);
				}
				search.pop();
			}
		}
		MoveGen(Board &target, Minos::Active &init, Piece &type)
			: target(target), data(&mino_cache[type]), up(up_offset[type]), down(down_offset[type]), left(left_offset[type]), right(right_offset[type]), type(type)
		{
			search.emplace(init, -1);
			search.front().y = std::min(search.front().y, target.y_max);
			coords.reserve(max_coords.load());
			states.reserve(max_states.load());
			result.reserve(max_states.load());
		}
		~MoveGen()
		{
			max_coords = std::max(max_coords.load(), coords.size());
			max_states = std::max(max_states.load(), states.size());
		}
	};
}