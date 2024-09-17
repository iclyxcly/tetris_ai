#pragma once
#include "board.hpp"
#include "mino.hpp"
#include "minotemplate.h"
#include <queue>
#include <vector>
#include <string>
namespace moenew
{
	class PathGen
	{
		struct MoveDataEx : public MoveData
		{
			std::string path;
			MoveDataEx() : MoveData() {}
			MoveDataEx(const MoveData &data) : MoveData(data) {}
		};
	public:
		Board target;
		const Minocache *data;
		const int *up;
		const int *down;
		const int *left;
		const int *right;
		Piece type;
		MoveData mino_target;
		MoveDataEx path_result;
		std::queue<MoveDataEx> search;
		std::vector<MoveDataEx> result;
		std::vector<int16_t> coords;
		std::vector<uint64_t> landpoints;
		constexpr uint64_t landpoint_hashify(const MoveDataEx &mino, const uint32_t data[4]) const
		{
			uint64_t hash = data[0];
			for (int i = 1; i < 4; ++i)
			{
				if (data[i] == 0)
				{
					continue;
				}
				hash = hash * 31 + data[i];
			}
			return hash * 7 + (mino.get_y() - down[mino.get_r()]);
		}
		bool try_push_coord(const MoveDataEx &mino)
		{
			auto hash = mino.hash();
			if (std::find(coords.rbegin(), coords.rend(), hash) == coords.rend())
			{
				coords.push_back(hash);
				search.push(mino);
				return true;
			}
			return false;
		}
		void try_push_landpoint(const MoveDataEx &mino)
		{
			auto hash = landpoint_hashify(mino, cache_get(type, mino.get_r(), mino.get_x()));
			if (std::find(landpoints.rbegin(), landpoints.rend(), hash) == landpoints.rend())
			{
				landpoints.push_back(hash);
				result.push_back(mino);
			}
			if (mino.hash() == mino_target.hash())
			{
				path_result = mino;
			}
		}
		bool integrate(const int8_t &x, const int8_t &y, const int8_t &r) const
		{
			const auto *rows = data->get(r, x);
			return target.integrate(rows, y);
		}
		bool test_up(const MoveDataEx &mino) const
		{
			return integrate(mino.get_x(), mino.get_y() + 1, mino.get_r());
		}
		bool test_down(const MoveDataEx &mino) const
		{
			int y = mino.get_y() - 1;
			return y >= down[mino.get_r()] && integrate(mino.get_x(), y, mino.get_r());
		}
		bool test_left(const MoveDataEx &mino) const
		{
			int x = mino.get_x() - 1;
			return x >= left[mino.get_r()] && integrate(x, mino.get_y(), mino.get_r());
		}
		bool test_right(const MoveDataEx &mino) const
		{
			int x = mino.get_x() + 1;
			return x <= target.w - right[mino.get_r()] - 4 && integrate(x, mino.get_y(), mino.get_r());
		}
		void harddrop(MoveDataEx &mino)
		{
			while (try_down(mino))
				;
			mino.set_y(mino.get_y() + 1);
			try_push_landpoint(mino);
		}
		bool try_down(MoveDataEx &mino) const
		{
			mino.set_y(mino.get_y() - 1);
			return mino.get_y() >= down[mino.get_r()] && integrate(mino.get_x(), mino.get_y(), mino.get_r());
		}
		bool try_left(MoveDataEx &mino) const
		{
			mino.set_x(mino.get_x() - 1);
			return mino.get_x() >= left[mino.get_r()] && integrate(mino.get_x(), mino.get_y(), mino.get_r());
		}
		bool try_right(MoveDataEx &mino) const
		{
			mino.set_x(mino.get_x() + 1);
			return mino.get_x() <= target.w - right[mino.get_r()] - 4 && integrate(mino.get_x(), mino.get_y(), mino.get_r());
		}
		bool try_cw(MoveDataEx &mino) const
		{
			mino.set_r((mino.get_r() + 1) & 3);
			const int rightmost = target.w - right[mino.get_r()] - 4;
			if (within(mino.get_x(), left[mino.get_r()], rightmost) && mino.get_y() >= down[mino.get_r()])
			{
				if (integrate(mino.get_x(), mino.get_y(), mino.get_r()))
				{
					return true;
				}
			}
			const auto *offset = clockwise(type, mino.get_r());
			const auto &max = clockwise_size(type, mino.get_r());
			for (int i = 0; i < max; i++)
			{
				int x = mino.get_x() + offset[i].x;
				int y = mino.get_y() + offset[i].y;
				if (within(x, left[mino.get_r()], rightmost) && y >= down[mino.get_r()])
				{
					if (integrate(x, y, mino.get_r()))
					{
						mino.set_x(x);
						mino.set_y(y);
						return true;
					}
				}
			}
			mino.set_r((mino.get_r() - 1) & 3);
			return false;
		}
		bool try_ccw(MoveDataEx &mino) const
		{
			mino.set_r((mino.get_r() - 1) & 3);
			const int rightmost = target.w - right[mino.get_r()] - 4;
			if (within(mino.get_x(), left[mino.get_r()], rightmost) && mino.get_y() >= down[mino.get_r()])
			{
				if (integrate(mino.get_x(), mino.get_y(), mino.get_r()))
				{
					return true;
				}
			}
			const auto *offset = counterclockwise(type, mino.get_r());
			const auto &max = counter_clockwise_size(type, mino.get_r());
			for (int i = 0; i < max; i++)
			{
				int x = mino.get_x() + offset[i].x;
				int y = mino.get_y() + offset[i].y;
				if (within(x, left[mino.get_r()], rightmost) && y >= down[mino.get_r()])
				{
					if (integrate(x, y, mino.get_r()))
					{
						mino.set_x(x);
						mino.set_y(y);
						return true;
					}
				}
			}
			mino.set_r((mino.get_r() + 1) & 3);
			return false;
		}
		bool try_180(MoveDataEx &mino) const
		{
			mino.set_r((mino.get_r() + 2) & 3);
			const int rightmost = target.w - right[mino.get_r()] - 4;
			if (within(mino.get_x(), left[mino.get_r()], rightmost) && mino.get_y() >= down[mino.get_r()])
			{
				if (integrate(mino.get_x(), mino.get_y(), mino.get_r()))
				{
					return true;
				}
			}
			const auto *offset = _180_spin(type, mino.get_r());
			const auto &max = _180_spin_size(type, mino.get_r());
			for (int i = 0; i < max; i++)
			{
				int x = mino.get_x() + offset[i].x;
				int y = mino.get_y() + offset[i].y;
				if (within(x, left[mino.get_r()], rightmost) && y >= down[mino.get_r()])
				{
					if (integrate(x, y, mino.get_r()))
					{
						mino.set_x(x);
						mino.set_y(y);
						return true;
					}
				}
			}
			mino.set_r((mino.get_r() + 2) & 3);
			return false;
		}
		void expand(MoveDataEx &node)
		{
			MoveDataEx copy = node;
			if (try_down(copy))
			{
				copy.path.push_back('d');
				try_push_coord(copy);
			}
			copy = node;
			if (try_left(copy))
			{
				copy.path.push_back('l');
				try_push_coord(copy);
			}
			copy = node;
			if (try_right(copy))
			{
				copy.path.push_back('r');
				try_push_coord(copy);
			}
			copy = node;
			if (try_cw(copy))
			{
				copy.path.push_back('c');
				try_push_coord(copy);
			}
			copy = node;
			if (try_ccw(copy))
			{
				copy.path.push_back('z');
				try_push_coord(copy);
			}
			harddrop(node);
		}
		std::string build()
		{
			while (!search.empty() && path_result.path.empty())
			{
				expand(search.front());
				search.pop();
			}
			return path_result.path;
		}
		void init(Board &target, MoveData loc, MoveData move_target, Piece type)
		{
			this->target = target;
			this->type = type;
			data = &mino_cache[type];
			up = up_offset[type];
			down = down_offset[type];
			left = left_offset[type];
			right = right_offset[type];
			mino_target = move_target;
			search.emplace(loc);
		}
		PathGen(Board &target, MoveData loc, MoveData move_target, Piece type)
		{
			init(target, loc, move_target, type);
		}
	};
}