#pragma once
#include <cstdint>
#include <random>
#include <string>
namespace moenew
{
	using rng_range = std::uniform_int_distribution<>;
	using random = std::mt19937;
	inline constexpr int BEAM_WIDTH = 1000;
	inline constexpr int BOARD_WIDTH = 10;
	inline constexpr int BOARD_HEIGHT = 40;
	inline constexpr int DEFAULT_X = 3;
	inline constexpr int DEFAULT_Y = 17;
	inline constexpr int DEFAULT_R = 0;
	enum Piece
	{
		S,
		L,
		Z,
		I,
		T,
		O,
		J,
		X
	};
	struct x_init
	{
		uint64_t data[64] = {};
		constexpr x_init()
		{
			for (int i = 0; i < 64; ++i)
			{
				data[i] = 1ull << i;
			}
		}
		inline constexpr const uint64_t &of(int i) const
		{
			return data[i];
		}
	};
	struct c_init
	{
		uint64_t data[64] = {};
		constexpr c_init()
		{
			for (int i = 0; i < 64; ++i)
			{
				data[i] |= (i - 1 < 0 ? 0 : data[i - 1]) + (1ull << i);
			}
		}
		inline constexpr const uint64_t &of(int i) const
		{
			return data[i];
		}
	};
	inline constexpr x_init loc_x;
	inline constexpr c_init loc_c;
}