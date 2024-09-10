#pragma once
#include <cstdint>
#include <array>
#include <string>
#include "const.h"
namespace moenew
{
	class Minos
	{
	public:
		struct Coord
		{
			int x;
			int y;
		};
		struct Active : Coord
		{
			int x;
			int y;
			int r;
		};
		struct MinoNode
		{
		private:
			uint8_t row[4];
			int up;
			int down;
			int left;
			int right;
			int _cw_max;
			int _ccw_max;
			int __180_max;
			Coord _cw[8];
			Coord _ccw[8];
			Coord __180[8];
		public:
			uint8_t* data();
			uint8_t& data(size_t i);
			uint8_t& data(int i);
			int& north();
			int& south();
			int& west();
			int& east();
			Coord& cw(size_t i);
			Coord& ccw(size_t i);
			Coord& _180(size_t i);
			Coord& cw(int i);
			Coord& ccw(int i);
			Coord& _180(int i);
			int& cw_max();
			int& ccw_max();
			int& _180_max();
		};
		struct Mino
		{
			MinoNode node[4];
			Mino();
			MinoNode& rot(size_t i);
			MinoNode& rot(int i);
		};
	private:
		inline static bool init = false;
		inline static Mino data[7];
	public:
		static Mino& get(size_t i);
		static Mino& get(int i);
		static void mark();
		static bool status();
		static void reset();
	};
	void init_minos(std::string path);
	class Minocache
	{
	private:
		uint32_t data[4][32][4];
		int left[4];
		int right[4];
	public:
		constexpr Minocache(const int Left[4], const int Right[4], const uint8_t Data[4][4])
		{
			for (int i = 0; i < 4; ++i)
			{
				left[i] = Left[i];
				right[i] = Right[i];
				for (int j = 0; j < 32; ++j)
				{
					for (int k = 0; k < 4; ++k)
					{
						data[i][j][k] = 0;
					}
				}
				for (int j = Left[i]; j < 29 - Right[i]; ++j)
				{
					for (int k = 0; k < 4; ++k)
					{
						if (j < 0)
						{
							data[i][j - Left[i]][k] = Data[i][k] >> -j;
						}
						else {
							data[i][j - Left[i]][k] = Data[i][k] << j;
						}
					}
				}
			}
		};
		inline constexpr const uint32_t* get(int r, int x) const
		{
			return data[r][x - left[r]];
		}
	};
}