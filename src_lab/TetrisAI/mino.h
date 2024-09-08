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
		struct Mino
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
			uint8_t* get();
			uint8_t& get(int& i);
			int& north();
			int& south();
			int& west();
			int& east();
			Coord& cw(int& i);
			Coord& ccw(int& i);
			Coord& _180(int& i);
			int& cw_max();
			int& ccw_max();
			int& _180_max();
		};
	private:
		inline static bool init = false;
		inline static Mino data[7];
	public:
		Mino& get(int& i);
		static void mark();
		static bool status();
		static void reset();
	};
	void init_minos(std::string& path);
}