#include "mino.h"
#include "const.h"
#include "json.hpp"
#include <fstream>
namespace moenew
{
	uint8_t* Minos::MinoNode::data() {
		return row;
	}

	uint8_t& Minos::MinoNode::data(size_t i) {
		return row[i];
	}

	uint8_t& Minos::MinoNode::data(int i) {
		return row[i];
	}

	int& Minos::MinoNode::north() {
		return up;
	}

	int& Minos::MinoNode::south() {
		return down;
	}

	int& Minos::MinoNode::west() {
		return left;
	}

	int& Minos::MinoNode::east() {
		return right;
	}

	typename Minos::Coord& Minos::MinoNode::cw(size_t i) {
		return _cw[i];
	}

	typename Minos::Coord& Minos::MinoNode::ccw(size_t i) {
		return _ccw[i];
	}

	typename Minos::Coord& Minos::MinoNode::_180(size_t i) {
		return __180[i];
	}

	typename Minos::Coord& Minos::MinoNode::cw(int i) {
		return _cw[i];
	}

	typename Minos::Coord& Minos::MinoNode::ccw(int i) {
		return _ccw[i];
	}

	typename Minos::Coord& Minos::MinoNode::_180(int i) {
		return __180[i];
	}

	int& Minos::MinoNode::cw_max() {
		return _cw_max;
	}

	int& Minos::MinoNode::ccw_max() {
		return _ccw_max;
	}

	int& Minos::MinoNode::_180_max() {
		return __180_max;
	}

	Minos::Mino::Mino() {
		memset(node, 0, sizeof(node));
	}

	typename Minos::MinoNode& Minos::Mino::rot(size_t i) {
		return node[i];
	}

	typename Minos::MinoNode& Minos::Mino::rot(int i)
	{
		return node[i];
	}

	typename Minos::Mino& Minos::get(size_t i) {
		return data[i];
	}

	typename Minos::Mino& Minos::get(int i)
	{
		return data[i];
	}

	void Minos::mark() {
		init = true;
	}

	void Minos::reset() {
		init = false;
		memset(data, 0, sizeof(data));
	}

	bool Minos::status() {
		return init;
	}

	void init_minos(std::string path) {
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
		for (auto& content : json) {
			const int &width = content["width"];
			const int &height = content["height"];
			size_t index = string_to_type(content["type"]);
			auto& mino = Minos::get(index);
			// i represents the rotation state, 0 to 3
			const size_t rots = content["data"].size();
			for (size_t i = 0; i < rots; i++)
			{
				// tetrimino data
				auto& mino_dir = mino.rot(i);
				for (size_t j = 0; j < height; j++)
				{
					size_t row_idx = 3 - j;
					auto& row = mino_dir.data(row_idx);
					for (size_t k = 0; k < width; k++)
					{
						row |= (content["data"][i][j][k] == 1) << k;
					}
				}
				// offset
				for (size_t j = 0; j < 4; j++)
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
					auto& coord = mino_dir.cw(j);
					coord.x = content["clockwise_kicks"][i][j][0];
					coord.y = content["clockwise_kicks"][i][j][1];
				}
				// counter-clockwise rotation
				const int ccw_coords = (int)content["counter_clockwise_kicks"][i].size();
				mino_dir.ccw_max() = ccw_coords;
				for (int j = 0; j < ccw_coords; j++)
				{
					auto& coord = mino_dir.ccw(j);
					coord.x = content["counter_clockwise_kicks"][i][j][0];
					coord.y = content["counter_clockwise_kicks"][i][j][1];
				}
				// 180-degree rotation
				const int _180_coords = (int)content["180_kicks"][i].size();
				mino_dir._180_max() = _180_coords;
				for (int j = 0; j < _180_coords; j++)
				{
					auto& coord = mino_dir._180(j);
					coord.x = content["180_kicks"][i][j][0];
					coord.y = content["180_kicks"][i][j][1];
				}
			}
		}
	}
}