#pragma once
#include <cstdint>
#include <random>
#include "utils.h"
namespace moenew
{
	using rng_range = std::uniform_int_distribution<>;
	using random = std::mt19937;
	inline constexpr int BOARD_WIDTH = 10;
	inline constexpr int BOARD_HEIGHT = 40;
	inline constexpr bool within(const int& x, const int& y, const int& w, const int& h)
	{
		return x >= 0 && x < w && y >= 0 && y < h;
	}
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
	inline constexpr uint64_t X_INDEX[64] = {
		0b1,
		0b10,
		0b100,
		0b1000,
		0b10000,
		0b100000,
		0b1000000,
		0b10000000,
		0b100000000,
		0b1000000000,
		0b10000000000,
		0b100000000000,
		0b1000000000000,
		0b10000000000000,
		0b100000000000000,
		0b1000000000000000,
		0b10000000000000000,
		0b100000000000000000,
		0b1000000000000000000,
		0b10000000000000000000,
		0b100000000000000000000,
		0b1000000000000000000000,
		0b10000000000000000000000,
		0b100000000000000000000000,
		0b1000000000000000000000000,
		0b10000000000000000000000000,
		0b100000000000000000000000000,
		0b1000000000000000000000000000,
		0b10000000000000000000000000000,
		0b100000000000000000000000000000,
		0b1000000000000000000000000000000,
		0b10000000000000000000000000000000,
		0b100000000000000000000000000000000,
		0b1000000000000000000000000000000000,
		0b10000000000000000000000000000000000,
		0b100000000000000000000000000000000000,
		0b1000000000000000000000000000000000000,
		0b10000000000000000000000000000000000000,
		0b100000000000000000000000000000000000000,
		0b1000000000000000000000000000000000000000,
		0b10000000000000000000000000000000000000000,
		0b100000000000000000000000000000000000000000,
		0b1000000000000000000000000000000000000000000,
		0b10000000000000000000000000000000000000000000,
		0b100000000000000000000000000000000000000000000,
		0b1000000000000000000000000000000000000000000000,
		0b10000000000000000000000000000000000000000000000,
		0b100000000000000000000000000000000000000000000000,
		0b1000000000000000000000000000000000000000000000000,
		0b10000000000000000000000000000000000000000000000000,
		0b100000000000000000000000000000000000000000000000000,
		0b1000000000000000000000000000000000000000000000000000,
		0b10000000000000000000000000000000000000000000000000000,
		0b100000000000000000000000000000000000000000000000000000,
		0b1000000000000000000000000000000000000000000000000000000,
		0b10000000000000000000000000000000000000000000000000000000,
		0b100000000000000000000000000000000000000000000000000000000,
		0b1000000000000000000000000000000000000000000000000000000000,
		0b10000000000000000000000000000000000000000000000000000000000,
		0b100000000000000000000000000000000000000000000000000000000000,
		0b1000000000000000000000000000000000000000000000000000000000000,
		0b10000000000000000000000000000000000000000000000000000000000000,
		0b100000000000000000000000000000000000000000000000000000000000000,
	};
	inline constexpr uint64_t LINE_CLEAR[64] = {
		0b1,
		0b11,
		0b111,
		0b1111,
		0b11111,
		0b111111,
		0b1111111,
		0b11111111,
		0b111111111,
		0b1111111111,
		0b11111111111,
		0b111111111111,
		0b1111111111111,
		0b11111111111111,
		0b111111111111111,
		0b1111111111111111,
		0b11111111111111111,
		0b111111111111111111,
		0b1111111111111111111,
		0b11111111111111111111,
		0b111111111111111111111,
		0b1111111111111111111111,
		0b11111111111111111111111,
		0b111111111111111111111111,
		0b1111111111111111111111111,
		0b11111111111111111111111111,
		0b111111111111111111111111111,
		0b1111111111111111111111111111,
		0b11111111111111111111111111111,
		0b111111111111111111111111111111,
		0b1111111111111111111111111111111,
		0b11111111111111111111111111111111,
		0b111111111111111111111111111111111,
		0b1111111111111111111111111111111111,
		0b11111111111111111111111111111111111,
		0b111111111111111111111111111111111111,
		0b1111111111111111111111111111111111111,
		0b11111111111111111111111111111111111111,
		0b111111111111111111111111111111111111111,
		0b1111111111111111111111111111111111111111,
		0b11111111111111111111111111111111111111111,
		0b111111111111111111111111111111111111111111,
		0b1111111111111111111111111111111111111111111,
		0b11111111111111111111111111111111111111111111,
		0b111111111111111111111111111111111111111111111,
		0b1111111111111111111111111111111111111111111111,
		0b11111111111111111111111111111111111111111111111,
		0b111111111111111111111111111111111111111111111111,
		0b1111111111111111111111111111111111111111111111111,
		0b11111111111111111111111111111111111111111111111111,
		0b111111111111111111111111111111111111111111111111111,
		0b1111111111111111111111111111111111111111111111111111,
		0b11111111111111111111111111111111111111111111111111111,
		0b111111111111111111111111111111111111111111111111111111,
		0b1111111111111111111111111111111111111111111111111111111,
		0b11111111111111111111111111111111111111111111111111111111,
		0b111111111111111111111111111111111111111111111111111111111,
		0b1111111111111111111111111111111111111111111111111111111111,
		0b11111111111111111111111111111111111111111111111111111111111,
		0b111111111111111111111111111111111111111111111111111111111111,
		0b1111111111111111111111111111111111111111111111111111111111111,
		0b11111111111111111111111111111111111111111111111111111111111111,
		0b111111111111111111111111111111111111111111111111111111111111111,
	};
}