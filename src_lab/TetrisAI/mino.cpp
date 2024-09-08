#include "mino.h"
#include "const.h"
#include "json.hpp"
#include <fstream>
namespace moenew
{
    uint8_t* Minos::Mino::get() {
        return row;
    }

	uint8_t& Minos::Mino::get(int& i) {
		return row[i];
	}

	int& Minos::Mino::north() {
		return up;
	}

	int& Minos::Mino::south() {
		return down;
	}

	int& Minos::Mino::west() {
		return left;
	}

	int& Minos::Mino::east() {
		return right;
	}

	typename Minos::Coord& Minos::Mino::cw(int& i) {
		return _cw[i];
	}

	typename Minos::Coord& Minos::Mino::ccw(int& i) {
		return _ccw[i];
	}

	typename Minos::Coord& Minos::Mino::_180(int& i) {
		return __180[i];
	}

	typename Minos::Mino& Minos::get(int& i) {
		return data[i];
	}

	int& Minos::Mino::cw_max() {
		return _cw_max;
	}

	int& Minos::Mino::ccw_max() {
		return _ccw_max;
	}

	int& Minos::Mino::_180_max() {
		return __180_max;
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

	void init_minos(std::string& path) {
		std::ifstream file(path);
		assert(file.good());
		nlohmann::json j;
		file >> j;
		file.close();
		j = j["minotypes"];
		if (Minos::status())
		{
			Minos::reset();
		}
		Minos mino_template;
		int nav = 0;
		for (auto& i : j) {
			auto& mino = mino_template.get(nav);
			auto& row = i["row"];
			for (int j = 0; j < 4; j++) {
				mino.get(j) = row[j];
			}
			mino.north() = i["north"];
			mino.south() = i["south"];
			mino.west() = i["west"];
			mino.east() = i["east"];
			auto& cw = i["cw"];
			auto& ccw = i["ccw"];
			auto& _180 = i["180"];
			for (int j = 0; j < 8; j++) {
				mino.cw(j).x = cw[j][0];
				mino.cw(j).y = cw[j][1];
				mino.ccw(j).x = ccw[j][0];
				mino.ccw(j).y = ccw[j][1];
				mino._180(j).x = _180[j][0];
				mino._180(j).y = _180[j][1];
			}
			mino.cw_max() = i["cw_max"];
			mino.ccw_max() = i["ccw_max"];
			mino._180_max() = i["180_max"];
			nav++;
		}
	}
}