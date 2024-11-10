#pragma once
#include "board.hpp"
#include "mino.hpp"
#include "minotemplate.h"
#include <queue>
#include <vector>
namespace moenew
{
	std::atomic<std::size_t> max_landpoints(0);
	class CoordTruthTable
	{
	public:
		uint32_t data[4][42];
		constexpr CoordTruthTable()
		{
			for (int r = 0; r < 4; r++)
			{
				for (int y = 0; y < 42; y++)
				{
					data[r][y] = false;
				}
			}
		}
		bool test(const int &r, const int &x, const int &y)
		{
			if ((data[r][y + 2] >> (x + 2)) & 1)
			{
				return false;
			}
			else
			{
				data[r][y + 2] |= 1 << (x + 2);
				return true;
			}
		}
	};
	class MoveGen
	{
	public:
		bool allow_search = true;
		Board target;
		const Minocache *data;
		const int *up;
		const int *down;
		const int *left;
		const int *right;
		char type;
		std::queue<MoveData> search;
		std::vector<MoveData> result;
		CoordTruthTable coords;
		std::vector<uint64_t> landpoints;
		constexpr uint64_t landpoint_hashify(const MoveData &mino, const uint32_t data[4]) const
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
		bool try_push_coord(const MoveData &mino)
		{
			auto hash = mino.hash();
			if (coords.test(mino.get_r(), mino.get_x(), mino.get_y()))
			{
				search.push(mino);
				return true;
			}
			return false;
		}
		void try_push_landpoint(const MoveData &mino)
		{
			auto hash = landpoint_hashify(mino, cache_get(type, mino.get_r(), mino.get_x()));
			if (std::find(landpoints.rbegin(), landpoints.rend(), hash) == landpoints.rend())
			{
				landpoints.push_back(hash);
				result.push_back(mino);
			}
		}
		bool integrate(const int8_t &x, const int8_t &y, const int8_t &r) const
		{
			const auto *rows = data->get(r, x);
			return target.integrate(rows, y);
		}
		bool test_up(const MoveData &mino) const
		{
			return integrate(mino.get_x(), mino.get_y() + 1, mino.get_r());
		}
		bool test_down(const MoveData &mino) const
		{
			int y = mino.get_y() - 1;
			return y >= down[mino.get_r()] && integrate(mino.get_x(), y, mino.get_r());
		}
		bool test_left(const MoveData &mino) const
		{
			int x = mino.get_x() - 1;
			return x >= left[mino.get_r()] && integrate(x, mino.get_y(), mino.get_r());
		}
		bool test_right(const MoveData &mino) const
		{
			int x = mino.get_x() + 1;
			return x <= target.w - right[mino.get_r()] - 4 && integrate(x, mino.get_y(), mino.get_r());
		}
		void harddrop(MoveData &mino)
		{
			while (try_down(mino))
				;
			mino.set_y(mino.get_y() + 1);
			try_push_landpoint(mino);
		}
		bool try_down(MoveData &mino) const
		{
			mino.set_y(mino.get_y() - 1);
			return mino.get_y() >= down[mino.get_r()] && integrate(mino.get_x(), mino.get_y(), mino.get_r());
		}
		bool try_left(MoveData &mino) const
		{
			mino.set_x(mino.get_x() - 1);
			return mino.get_x() >= left[mino.get_r()] && integrate(mino.get_x(), mino.get_y(), mino.get_r());
		}
		bool try_right(MoveData &mino) const
		{
			mino.set_x(mino.get_x() + 1);
			return mino.get_x() <= target.w - right[mino.get_r()] - 4 && integrate(mino.get_x(), mino.get_y(), mino.get_r());
		}
		bool try_cw(MoveData &mino) const
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
		bool try_ccw(MoveData &mino) const
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
		bool try_180(MoveData &mino) const
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
		bool immobile(const MoveData &mino) const
		{
			return !test_up(mino) && !test_down(mino) && !test_left(mino) && !test_right(mino);
		}
		static bool immobile_global(const MoveData &mino, const char &type, const Board &board)
		{
			const Minocache *data = &mino_cache[type];
			const int *up = up_offset[type];
			const int *down = down_offset[type];
			const int *left = left_offset[type];
			const int *right = right_offset[type];
			auto integrate = [&board, &data](const int8_t &x, const int8_t &y, const int8_t &r) -> bool
			{
				const auto *rows = data->get(r, x);
				return board.integrate(rows, y);
			};
			auto test_up = [&integrate](const MoveData &mino) -> bool
			{
				return integrate(mino.get_x(), mino.get_y() + 1, mino.get_r());
			};
			auto test_down = [&integrate, &down](const MoveData &mino) -> bool
			{
				int y = mino.get_y() - 1;
				return y >= down[mino.get_r()] && integrate(mino.get_x(), y, mino.get_r());
			};
			auto test_left = [&integrate, &left](const MoveData &mino) -> bool
			{
				int x = mino.get_x() - 1;
				return x >= left[mino.get_r()] && integrate(x, mino.get_y(), mino.get_r());
			};
			auto test_right = [&integrate, &right, &board](const MoveData &mino) -> bool
			{
				int x = mino.get_x() + 1;
				return x <= board.w - right[mino.get_r()] - 4 && integrate(x, mino.get_y(), mino.get_r());
			};
			return !test_up(mino) && !test_down(mino) && !test_left(mino) && !test_right(mino);
		}
		bool allspin(const MoveData &mino) const
		{
			return mino.get_status() == Rotate && !test_up(mino) && !test_down(mino) && !test_left(mino) && !test_right(mino);
		}
		void expand(MoveData &node)
		{
			MoveData copy = node;
			auto last = copy.get_status();
			if (last != Down)
			{
				copy.set_status(Down);
				while (try_down(copy) && try_push_coord(copy))
					;
				harddrop(node);
				node.set_status(Rotate);
				copy = node;
				if (try_cw(copy))
				{
					try_push_coord(copy);
					copy = node;
				}
				if (try_ccw(copy))
				{
					try_push_coord(copy);
				}
			}
			if (last != LR)
			{
				node.set_status(LR);
				copy = node;
				while (try_left(copy) && try_push_coord(copy))
					;
				copy = node;
				while (try_right(copy) && try_push_coord(copy))
					;
			}
		}
		void start()
		{
			if (!allow_search)
			{
				return;
			}
			while (!search.empty())
			{
				expand(search.front());
				search.pop();
			}
		}
		void init(Board &target, MoveData loc, char type)
		{
			if (target.y_max >= loc.get_y())
			{
				auto *rows = cache_get(type, loc.get_r(), loc.get_x());
				if (!target.integrate(rows, loc.get_y()))
				{
					allow_search = false;
					return;
				}
			}
			this->target = target;
			this->type = type;
			data = &mino_cache[type];
			up = up_offset[type];
			down = down_offset[type];
			left = left_offset[type];
			right = right_offset[type];
			search.emplace(loc);
			search.front().set_y(std::min<int>(search.front().get_y(), target.y_max));
			landpoints.reserve(max_landpoints);
			result.reserve(max_landpoints);
		}
		MoveGen(Board &target, MoveData loc, char type)
		{
			init(target, loc, type);
		}
		MoveGen() {};
		~MoveGen()
		{
			max_landpoints = std::max(max_landpoints.load(), landpoints.size());
		}
	};
}
