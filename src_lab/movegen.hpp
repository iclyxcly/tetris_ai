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
	std::atomic<std::size_t> max_landpoints(0);
	class MoveGen
	{
	public:
		Board &target;
		const Minocache *data;
		const int *up;
		const int *down;
		const int *left;
		const int *right;
		Piece &type;
		std::queue<Minos::Active> search;
		std::vector<Minos::Active> result;
		std::unordered_set<uint16_t> coords;
		std::unordered_set<uint64_t> landpoints;
		constexpr uint16_t hashify(const Minos::Active &mino) const
		{
			return (mino.x + 2) + 34 * ((mino.y + 2) + 32 * mino.r);
		}
		constexpr uint64_t landpoint_hashify(const Minos::Active &mino, const uint32_t data[4])
		{
			return data[0] * 7 + data[1] * 7 + data[2] * 7 + data[3] * 7 + (mino.y - down[mino.r]);
		}
		bool try_push_coord(const Minos::Active &mino)
		{
			auto hash = hashify(mino);
			if (coords.find(hash) == coords.end())
			{
				coords.insert(hash);
				search.push(mino);
				return true;
			}
			return false;
		}
		void try_push_landpoint(Minos::Active &mino)
		{
			while (try_down(mino));
			++mino.y;
			auto hash = landpoint_hashify(mino, cache_get(type, mino.r, mino.x));
			if (landpoints.find(hash) == landpoints.end())
			{
				landpoints.insert(hash);
				result.push_back(mino);
			}
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
		bool try_down(Minos::Active &mino) const
		{
			--mino.y;
			return mino.y >= down[mino.r] && integrate(mino.x, mino.y, mino.r);
		}
		bool try_left(Minos::Active &mino) const
		{
			--mino.x;
			return mino.x >= left[mino.r] && integrate(mino.x, mino.y, mino.r);
		}
		bool try_right(Minos::Active &mino) const
		{
			++mino.x;
			return mino.x <= target.w - right[mino.r] - 4 && integrate(mino.x, mino.y, mino.r);
		}
		bool try_cw(Minos::Active &mino) const
		{
			mino.r = (mino.r + 1) & 3;
			int rightmost = target.w - right[mino.r] - 4;
			if (within(mino.x, left[mino.r], rightmost) && mino.y >= down[mino.r])
			{
				if (integrate(mino.x, mino.y, mino.r))
				{
					mino.last_rotate = true;
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
						mino.x = x;
						mino.y = y;
						mino.last_rotate = true;
						return true;
					}
				}
			}
			mino.r = (mino.r - 1) & 3;
			return false;
		}
		bool try_ccw(Minos::Active &mino) const
		{
			mino.r = (mino.r - 1) & 3;
			int rightmost = target.w - right[mino.r] - 4;
			if (within(mino.x, left[mino.r], rightmost) && mino.y >= down[mino.r])
			{
				if (integrate(mino.x, mino.y, mino.r))
				{
					mino.last_rotate = true;
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
						mino.x = x;
						mino.y = y;
						mino.last_rotate = true;
						return true;
					}
				}
			}
			mino.r = (mino.r + 1) & 3;
			return false;
		}
		bool try_180(Minos::Active &mino) const
		{
			mino.r = (mino.r + 2) & 3;
			int rightmost = target.w - right[mino.r] - 4;
			if (within(mino.x, left[mino.r], rightmost) && mino.y >= down[mino.r])
			{
				if (integrate(mino.x, mino.y, mino.r))
				{
					mino.last_rotate = true;
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
						mino.x = x;
						mino.y = y;
						mino.last_rotate = true;
						return true;
					}
				}
			}
			mino.r = (mino.r + 2) & 3;
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
		void expand(Minos::Active &node)
		{
			Minos::Active copy = node;
			while (try_left(copy) && try_push_coord(copy));
			copy.x = node.x;
			while (try_right(copy) && try_push_coord(copy));
			copy.x = node.x;
			while (try_down(copy) && try_push_coord(copy));
			copy.y = node.y;
			if (try_cw(copy))
			{
				try_push_coord(copy);
				copy = node;
			}
			if (try_ccw(copy))
			{
				try_push_coord(copy);
				copy = node;
			}
			if (try_180(copy))
			{
				try_push_coord(copy);
			}
		}
		void start()
		{
			while (!search.empty())
			{
				auto &node = search.front();
				expand(node);
				try_push_landpoint(node);
				search.pop();
			}
		}
		MoveGen(Board &target, Minos::Active &init, Piece &type)
			: target(target), data(&mino_cache[type]), up(up_offset[type]), down(down_offset[type]), left(left_offset[type]), right(right_offset[type]), type(type)
		{
			search.emplace(init);
			search.front().y = std::min(search.front().y, target.y_max);
			coords.reserve(max_coords);
			landpoints.reserve(max_landpoints);
			result.reserve(max_landpoints);
		}
		~MoveGen()
		{
			max_coords = std::max(max_coords.load(), coords.size());
			max_landpoints = std::max(max_landpoints.load(), landpoints.size());
		}
	};
}