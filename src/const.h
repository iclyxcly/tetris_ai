#pragma once
#include <cstdint>
#include <random>
#include <string>
namespace moenew
{
	inline constexpr int BEAM_WIDTH = 2000;
	inline constexpr int BOARD_WIDTH = 10;
	inline constexpr int BOARD_HEIGHT = 31;
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
	} loc_x;
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
	} loc_c;
}