#pragma once
#include <cstdint>
#include <array>
#include <string>
#include <cstring>
#include <fstream>
#include "const.h"
#include "utils.hpp"
#include "json.hpp"
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
			uint8_t *data()
			{
				return row;
			}
			uint8_t &data(std::size_t i)
			{
				return row[i];
			}
			uint8_t &data(int i)
			{
				return row[i];
			}
			int &north()
			{
				return up;
			}
			int &south()
			{
				return down;
			}
			int &west()
			{
				return left;
			}
			int &east()
			{
				return right;
			}
			Coord &cw(std::size_t i)
			{
				return _cw[i];
			}
			Coord &ccw(std::size_t i)
			{
				return _ccw[i];
			}
			Coord &_180(std::size_t i)
			{
				return __180[i];
			}
			Coord &cw(int i)
			{
				return _cw[i];
			}
			Coord &ccw(int i)
			{
				return _ccw[i];
			}
			Coord &_180(int i)
			{
				return __180[i];
			}
			int &cw_max()
			{
				return _cw_max;
			}
			int &ccw_max()
			{
				return _ccw_max;
			}
			int &_180_max()
			{
				return __180_max;
			}
		};
		struct Mino
		{
			MinoNode node[4];
			Mino()
			{
				std::memset(node, 0, sizeof(node));
			}
			MinoNode &rot(std::size_t i)
			{
				return node[i];
			}
			MinoNode &rot(int i)
			{
				return node[i];
			}
		};

	private:
		inline static bool init = false;
		static Mino data[7];

	public:
		static Mino &get(std::size_t i)
		{
			return data[i];
		}
		static Mino &get(int i)
		{
			return data[i];
		}
		static void mark()
		{
			init = true;
		}
		static const bool &status()
		{
			return init;
		}
		static void reset()
		{
			init = false;
			std::memset(data, 0, sizeof(data));
		}
	};
	void init_minos(std::string path)
	{

		std::ifstream file(path);
		assert(file.is_open());
		nlohmann::json json;
		file >> json;
		file.close();
		json = json["minotypes"];
		if (Minos::status())
		{
			Minos::reset();
		}
		for (auto &content : json)
		{
			const int &width = content["width"];
			const int &height = content["height"];
			std::string type = content["type"];
			std::size_t index = char_to_type(type[0]);
			auto &mino = Minos::get(index);
			// i represents the rotation state, 0 to 3
			const std::size_t rots = content["data"].size();
			for (std::size_t i = 0; i < rots; i++)
			{
				// tetrimino data
				auto &mino_dir = mino.rot(i);
				for (std::size_t j = 0; j < height; j++)
				{
					std::size_t row_idx = 3 - j;
					auto &row = mino_dir.data(row_idx);
					for (std::size_t k = 0; k < width; k++)
					{
						row |= (content["data"][i][j][k] == 1) << k;
					}
				}
				// offset
				for (std::size_t j = 0; j < 4; j++)
				{
					mino_dir.north() = content["offset"][i][0];
					mino_dir.east() = content["offset"][i][1];
					mino_dir.south() = content["offset"][i][2];
					mino_dir.west() = content["offset"][i][3];
				}
				// clockwise rotation
				const int cw_coords = (int)content["clockwise_kicks"][i].size();
				mino_dir.cw_max() = cw_coords;
				for (int j = 0; j < cw_coords; j++)
				{
					auto &coord = mino_dir.cw(j);
					coord.x = content["clockwise_kicks"][i][j][0];
					coord.y = content["clockwise_kicks"][i][j][1];
				}
				// counter-clockwise rotation
				const int ccw_coords = (int)content["counter_clockwise_kicks"][i].size();
				mino_dir.ccw_max() = ccw_coords;
				for (int j = 0; j < ccw_coords; j++)
				{
					auto &coord = mino_dir.ccw(j);
					coord.x = content["counter_clockwise_kicks"][i][j][0];
					coord.y = content["counter_clockwise_kicks"][i][j][1];
				}
				// 180-degree rotation
				const int _180_coords = (int)content["180_kicks"][i].size();
				mino_dir._180_max() = _180_coords;
				for (int j = 0; j < _180_coords; j++)
				{
					auto &coord = mino_dir._180(j);
					coord.x = content["180_kicks"][i][j][0];
					coord.y = content["180_kicks"][i][j][1];
				}
			}
		}
	}
	class Minocache
	{
	private:
		uint32_t data[4][32][4] = {};
		int left[4] = {};
		int right[4] = {};
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
							data[i][j - Left[i]][k] = static_cast<uint32_t>(Data[i][k]) >> -j;
						}
						else
						{
							data[i][j - Left[i]][k] = static_cast<uint32_t>(Data[i][k]) << j;
						}
					}
				}
			}
		}

		inline constexpr const uint32_t *get(int r, int x) const
		{
			return data[r][x - left[r]];
		}
	};
	Minos::Mino Minos::data[7];
}