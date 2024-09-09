#include "board.h"
#include "mino.h"
#include "output.h"
#include <map>
#pragma warning(disable: 4996)
int main()
{
	std::string file = "botris_srs.json";
	moenew::init_minos(file);

	std::map<size_t, std::string> type_to_string_cw = {
		{0, "L->0"},
		{1, "0->R"},
		{2, "R->2"},
		{3, "2->L"} };

	std::map<size_t, std::string> type_to_string_ccw = {
		{0, "R->0"},
		{1, "2->R"},
		{2, "L->2"},
		{3, "0->L"} };

	for (size_t i = 0; i < 7; ++i)
	{
		auto& mino = moenew::Minos::get(i);
		printf("Type: %c\n", moenew::type_to_char(i));
		for (size_t j = 0; j < 4; ++j)
		{
			auto& node = mino.rot(j);
			for (int k = 3; k >= 0; --k)
			{
				printf("|");
				for (int l = 0; l < 4; ++l)
				{
					printf("%s", (node.data(k) & moenew::X_INDEX[l]) ? "[]" : "  ");
				}
				printf("|\n");
			}
			printf("\nCW:\n");
			printf("%s: ", type_to_string_cw[j].c_str());
			for (int i = 0; i < node.cw_max(); ++i)
			{
				printf("(%d, %d) ", node.cw(i).x, node.cw(i).y);
			}
			printf("\nCCW:\n");
			printf("%s: ", type_to_string_ccw[j].c_str());
			for (int i = 0; i < node.ccw_max(); ++i)
			{
				printf("(%d, %d) ", node.ccw(i).x, node.ccw(i).y);
			}
			printf("\n\nUp: %d, Down: %d, Left: %d, Right: %d", node.north(), node.south(), node.west(), node.east());
			printf("\n--------------------------------------------\n");
		}
	}

	printf("Please verify if the data is correct. Press CTRL+C to cancel, enter to export as header file.\n");
	getchar();
	FILE* out = fopen("output.h", "w");
	auto& s_mino = moenew::Minos::get(moenew::S);
	auto& l_mino = moenew::Minos::get(moenew::L);
	auto& z_mino = moenew::Minos::get(moenew::Z);
	auto& i_mino = moenew::Minos::get(moenew::I);
	auto& t_mino = moenew::Minos::get(moenew::T);
	auto& o_mino = moenew::Minos::get(moenew::O);
	auto& j_mino = moenew::Minos::get(moenew::J);
	fprintf(out, "#pragma once\n");
	fprintf(out, "#include <cstdint>\n");
	fprintf(out, "#include <vector>\n");
	fprintf(out, "#include \"mino.h\"\n");
	fprintf(out, "// THIS FILE IS NOT MEANT TO BE READ !!! PLEASE GENERATE A MINO CONFIG FILE USING JSON2HEADER\n");
	fprintf(out, "namespace moenew {\n");
	fprintf(out, "	constexpr uint8_t minodata[7][4][4] = {\n");
	fprintf(out, "		{{%hhu, %hhu, %hhu, %hhu},\n", s_mino.rot(0).data(0), s_mino.rot(0).data(1), s_mino.rot(0).data(2), s_mino.rot(0).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu},\n", s_mino.rot(1).data(0), s_mino.rot(1).data(1), s_mino.rot(1).data(2), s_mino.rot(1).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu},\n", s_mino.rot(2).data(0), s_mino.rot(2).data(1), s_mino.rot(2).data(2), s_mino.rot(2).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu}},\n", s_mino.rot(3).data(0), s_mino.rot(3).data(1), s_mino.rot(3).data(2), s_mino.rot(3).data(3));
	fprintf(out, "		{{%hhu, %hhu, %hhu, %hhu},\n", l_mino.rot(0).data(0), l_mino.rot(0).data(1), l_mino.rot(0).data(2), l_mino.rot(0).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu},\n", l_mino.rot(1).data(0), l_mino.rot(1).data(1), l_mino.rot(1).data(2), l_mino.rot(1).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu},\n", l_mino.rot(2).data(0), l_mino.rot(2).data(1), l_mino.rot(2).data(2), l_mino.rot(2).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu}},\n", l_mino.rot(3).data(0), l_mino.rot(3).data(1), l_mino.rot(3).data(2), l_mino.rot(3).data(3));
	fprintf(out, "		{{%hhu, %hhu, %hhu, %hhu},\n", z_mino.rot(0).data(0), z_mino.rot(0).data(1), z_mino.rot(0).data(2), z_mino.rot(0).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu},\n", z_mino.rot(1).data(0), z_mino.rot(1).data(1), z_mino.rot(1).data(2), z_mino.rot(1).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu},\n", z_mino.rot(2).data(0), z_mino.rot(2).data(1), z_mino.rot(2).data(2), z_mino.rot(2).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu}},\n", z_mino.rot(3).data(0), z_mino.rot(3).data(1), z_mino.rot(3).data(2), z_mino.rot(3).data(3));
	fprintf(out, "		{{%hhu, %hhu, %hhu, %hhu},\n", i_mino.rot(0).data(0), i_mino.rot(0).data(1), i_mino.rot(0).data(2), i_mino.rot(0).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu},\n", i_mino.rot(1).data(0), i_mino.rot(1).data(1), i_mino.rot(1).data(2), i_mino.rot(1).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu},\n", i_mino.rot(2).data(0), i_mino.rot(2).data(1), i_mino.rot(2).data(2), i_mino.rot(2).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu}},\n", i_mino.rot(3).data(0), i_mino.rot(3).data(1), i_mino.rot(3).data(2), i_mino.rot(3).data(3));
	fprintf(out, "		{{%hhu, %hhu, %hhu, %hhu},\n", t_mino.rot(0).data(0), t_mino.rot(0).data(1), t_mino.rot(0).data(2), t_mino.rot(0).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu},\n", t_mino.rot(1).data(0), t_mino.rot(1).data(1), t_mino.rot(1).data(2), t_mino.rot(1).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu},\n", t_mino.rot(2).data(0), t_mino.rot(2).data(1), t_mino.rot(2).data(2), t_mino.rot(2).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu}},\n", t_mino.rot(3).data(0), t_mino.rot(3).data(1), t_mino.rot(3).data(2), t_mino.rot(3).data(3));
	fprintf(out, "		{{%hhu, %hhu, %hhu, %hhu},\n", o_mino.rot(0).data(0), o_mino.rot(0).data(1), o_mino.rot(0).data(2), o_mino.rot(0).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu},\n", o_mino.rot(1).data(0), o_mino.rot(1).data(1), o_mino.rot(1).data(2), o_mino.rot(1).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu},\n", o_mino.rot(2).data(0), o_mino.rot(2).data(1), o_mino.rot(2).data(2), o_mino.rot(2).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu}},\n", o_mino.rot(3).data(0), o_mino.rot(3).data(1), o_mino.rot(3).data(2), o_mino.rot(3).data(3));
	fprintf(out, "		{{%hhu, %hhu, %hhu, %hhu},\n", j_mino.rot(0).data(0), j_mino.rot(0).data(1), j_mino.rot(0).data(2), j_mino.rot(0).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu},\n", j_mino.rot(1).data(0), j_mino.rot(1).data(1), j_mino.rot(1).data(2), j_mino.rot(1).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu},\n", j_mino.rot(2).data(0), j_mino.rot(2).data(1), j_mino.rot(2).data(2), j_mino.rot(2).data(3));
	fprintf(out, "		{%hhu, %hhu, %hhu, %hhu}}\n", j_mino.rot(3).data(0), j_mino.rot(3).data(1), j_mino.rot(3).data(2), j_mino.rot(3).data(3));
	fprintf(out, "	};\n");
	fprintf(out, "	constexpr int up_offset[7][4] = {\n");
	fprintf(out, "		{%d, %d, %d, %d},\n", s_mino.rot(0).north(), s_mino.rot(1).north(), s_mino.rot(2).north(), s_mino.rot(2).north());
	fprintf(out, "		{%d, %d, %d, %d},\n", l_mino.rot(0).north(), l_mino.rot(1).north(), l_mino.rot(2).north(), l_mino.rot(2).north());
	fprintf(out, "		{%d, %d, %d, %d},\n", z_mino.rot(0).north(), z_mino.rot(1).north(), z_mino.rot(2).north(), z_mino.rot(2).north());
	fprintf(out, "		{%d, %d, %d, %d},\n", i_mino.rot(0).north(), i_mino.rot(1).north(), i_mino.rot(2).north(), i_mino.rot(2).north());
	fprintf(out, "		{%d, %d, %d, %d},\n", t_mino.rot(0).north(), t_mino.rot(1).north(), t_mino.rot(2).north(), t_mino.rot(2).north());
	fprintf(out, "		{%d, %d, %d, %d},\n", o_mino.rot(0).north(), o_mino.rot(1).north(), o_mino.rot(2).north(), o_mino.rot(2).north());
	fprintf(out, "		{%d, %d, %d, %d}\n", j_mino.rot(0).north(), j_mino.rot(1).north(), j_mino.rot(2).north(), j_mino.rot(2).north());
	fprintf(out, "	};\n");
	fprintf(out, "	constexpr int down_offset[7][4] = {\n");
	fprintf(out, "		{%d, %d, %d, %d},\n", s_mino.rot(0).south(), s_mino.rot(1).south(), s_mino.rot(2).south(), s_mino.rot(2).south());
	fprintf(out, "		{%d, %d, %d, %d},\n", l_mino.rot(0).south(), l_mino.rot(1).south(), l_mino.rot(2).south(), l_mino.rot(2).south());
	fprintf(out, "		{%d, %d, %d, %d},\n", z_mino.rot(0).south(), z_mino.rot(1).south(), z_mino.rot(2).south(), z_mino.rot(2).south());
	fprintf(out, "		{%d, %d, %d, %d},\n", i_mino.rot(0).south(), i_mino.rot(1).south(), i_mino.rot(2).south(), i_mino.rot(2).south());
	fprintf(out, "		{%d, %d, %d, %d},\n", t_mino.rot(0).south(), t_mino.rot(1).south(), t_mino.rot(2).south(), t_mino.rot(2).south());
	fprintf(out, "		{%d, %d, %d, %d},\n", o_mino.rot(0).south(), o_mino.rot(1).south(), o_mino.rot(2).south(), o_mino.rot(2).south());
	fprintf(out, "		{%d, %d, %d, %d}\n", j_mino.rot(0).south(), j_mino.rot(1).south(), j_mino.rot(2).south(), j_mino.rot(2).south());
	fprintf(out, "	};\n");
	fprintf(out, "	constexpr int left_offset[7][4] = {\n");
	fprintf(out, "		{%d, %d, %d, %d},\n", s_mino.rot(0).west(), s_mino.rot(1).west(), s_mino.rot(2).west(), s_mino.rot(2).west());
	fprintf(out, "		{%d, %d, %d, %d},\n", l_mino.rot(0).west(), l_mino.rot(1).west(), l_mino.rot(2).west(), l_mino.rot(2).west());
	fprintf(out, "		{%d, %d, %d, %d},\n", z_mino.rot(0).west(), z_mino.rot(1).west(), z_mino.rot(2).west(), z_mino.rot(2).west());
	fprintf(out, "		{%d, %d, %d, %d},\n", i_mino.rot(0).west(), i_mino.rot(1).west(), i_mino.rot(2).west(), i_mino.rot(2).west());
	fprintf(out, "		{%d, %d, %d, %d},\n", t_mino.rot(0).west(), t_mino.rot(1).west(), t_mino.rot(2).west(), t_mino.rot(2).west());
	fprintf(out, "		{%d, %d, %d, %d},\n", o_mino.rot(0).west(), o_mino.rot(1).west(), o_mino.rot(2).west(), o_mino.rot(2).west());
	fprintf(out, "		{%d, %d, %d, %d}\n", j_mino.rot(0).west(), j_mino.rot(1).west(), j_mino.rot(2).west(), j_mino.rot(2).west());
	fprintf(out, "	};\n");
	fprintf(out, "	constexpr int right_offset[7][4] = {\n");
	fprintf(out, "		{%d, %d, %d, %d},\n", s_mino.rot(0).east(), s_mino.rot(1).east(), s_mino.rot(2).east(), s_mino.rot(2).east());
	fprintf(out, "		{%d, %d, %d, %d},\n", l_mino.rot(0).east(), l_mino.rot(1).east(), l_mino.rot(2).east(), l_mino.rot(2).east());
	fprintf(out, "		{%d, %d, %d, %d},\n", z_mino.rot(0).east(), z_mino.rot(1).east(), z_mino.rot(2).east(), z_mino.rot(2).east());
	fprintf(out, "		{%d, %d, %d, %d},\n", i_mino.rot(0).east(), i_mino.rot(1).east(), i_mino.rot(2).east(), i_mino.rot(2).east());
	fprintf(out, "		{%d, %d, %d, %d},\n", t_mino.rot(0).east(), t_mino.rot(1).east(), t_mino.rot(2).east(), t_mino.rot(2).east());
	fprintf(out, "		{%d, %d, %d, %d},\n", o_mino.rot(0).east(), o_mino.rot(1).east(), o_mino.rot(2).east(), o_mino.rot(2).east());
	fprintf(out, "		{%d, %d, %d, %d}\n", j_mino.rot(0).east(), j_mino.rot(1).east(), j_mino.rot(2).east(), j_mino.rot(2).east());
	fprintf(out, "	};\n");
	fprintf(out, "	constexpr Minocache s_cache(left_offset[S], right_offset[S], minodata[S]);\n");
	fprintf(out, "	constexpr Minocache z_cache(left_offset[Z], right_offset[Z], minodata[Z]);\n");
	fprintf(out, "	constexpr Minocache l_cache(left_offset[L], right_offset[L], minodata[L]);\n");
	fprintf(out, "	constexpr Minocache j_cache(left_offset[J], right_offset[J], minodata[J]);\n");
	fprintf(out, "	constexpr Minocache i_cache(left_offset[I], right_offset[I], minodata[I]);\n");
	fprintf(out, "	constexpr Minocache o_cache(left_offset[O], right_offset[O], minodata[O]);\n");
	fprintf(out, "	constexpr Minocache t_cache(left_offset[T], right_offset[T], minodata[T]);\n");
	fprintf(out, "	constexpr Minocache mino_cache[7] = {s_cache, z_cache, l_cache, j_cache, i_cache, o_cache, t_cache};\n");
	fprintf(out, "	inline constexpr const uint32_t* cache_get(int m, int r, int x) {\n");
	fprintf(out, "		return mino_cache[m].get(r, x);\n");
	fprintf(out, "	}\n");
	fprintf(out, "	constexpr Minos::Coord cw_offset[7][4][8] = {\n");
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", s_mino.rot(0).cw(0).x, s_mino.rot(0).cw(0).y, s_mino.rot(0).cw(1).x, s_mino.rot(0).cw(1).y, s_mino.rot(0).cw(2).x, s_mino.rot(0).cw(2).y, s_mino.rot(0).cw(3).x, s_mino.rot(0).cw(3).y, s_mino.rot(0).cw(4).x, s_mino.rot(0).cw(4).y, s_mino.rot(0).cw(5).x, s_mino.rot(0).cw(5).y, s_mino.rot(0).cw(6).x, s_mino.rot(0).cw(6).y, s_mino.rot(0).cw(7).x, s_mino.rot(0).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", s_mino.rot(1).cw(0).x, s_mino.rot(1).cw(0).y, s_mino.rot(1).cw(1).x, s_mino.rot(1).cw(1).y, s_mino.rot(1).cw(2).x, s_mino.rot(1).cw(2).y, s_mino.rot(1).cw(3).x, s_mino.rot(1).cw(3).y, s_mino.rot(1).cw(4).x, s_mino.rot(1).cw(4).y, s_mino.rot(1).cw(5).x, s_mino.rot(1).cw(5).y, s_mino.rot(1).cw(6).x, s_mino.rot(1).cw(6).y, s_mino.rot(1).cw(7).x, s_mino.rot(1).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", s_mino.rot(2).cw(0).x, s_mino.rot(2).cw(0).y, s_mino.rot(2).cw(1).x, s_mino.rot(2).cw(1).y, s_mino.rot(2).cw(2).x, s_mino.rot(2).cw(2).y, s_mino.rot(2).cw(3).x, s_mino.rot(2).cw(3).y, s_mino.rot(2).cw(4).x, s_mino.rot(2).cw(4).y, s_mino.rot(2).cw(5).x, s_mino.rot(2).cw(5).y, s_mino.rot(2).cw(6).x, s_mino.rot(2).cw(6).y, s_mino.rot(2).cw(7).x, s_mino.rot(2).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", s_mino.rot(3).cw(0).x, s_mino.rot(3).cw(0).y, s_mino.rot(3).cw(1).x, s_mino.rot(3).cw(1).y, s_mino.rot(3).cw(2).x, s_mino.rot(3).cw(2).y, s_mino.rot(3).cw(3).x, s_mino.rot(3).cw(3).y, s_mino.rot(3).cw(4).x, s_mino.rot(3).cw(4).y, s_mino.rot(3).cw(5).x, s_mino.rot(3).cw(5).y, s_mino.rot(3).cw(6).x, s_mino.rot(3).cw(6).y, s_mino.rot(3).cw(7).x, s_mino.rot(3).cw(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", l_mino.rot(0).cw(0).x, l_mino.rot(0).cw(0).y, l_mino.rot(0).cw(1).x, l_mino.rot(0).cw(1).y, l_mino.rot(0).cw(2).x, l_mino.rot(0).cw(2).y, l_mino.rot(0).cw(3).x, l_mino.rot(0).cw(3).y, l_mino.rot(0).cw(4).x, l_mino.rot(0).cw(4).y, l_mino.rot(0).cw(5).x, l_mino.rot(0).cw(5).y, l_mino.rot(0).cw(6).x, l_mino.rot(0).cw(6).y, l_mino.rot(0).cw(7).x, l_mino.rot(0).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", l_mino.rot(1).cw(0).x, l_mino.rot(1).cw(0).y, l_mino.rot(1).cw(1).x, l_mino.rot(1).cw(1).y, l_mino.rot(1).cw(2).x, l_mino.rot(1).cw(2).y, l_mino.rot(1).cw(3).x, l_mino.rot(1).cw(3).y, l_mino.rot(1).cw(4).x, l_mino.rot(1).cw(4).y, l_mino.rot(1).cw(5).x, l_mino.rot(1).cw(5).y, l_mino.rot(1).cw(6).x, l_mino.rot(1).cw(6).y, l_mino.rot(1).cw(7).x, l_mino.rot(1).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", l_mino.rot(2).cw(0).x, l_mino.rot(2).cw(0).y, l_mino.rot(2).cw(1).x, l_mino.rot(2).cw(1).y, l_mino.rot(2).cw(2).x, l_mino.rot(2).cw(2).y, l_mino.rot(2).cw(3).x, l_mino.rot(2).cw(3).y, l_mino.rot(2).cw(4).x, l_mino.rot(2).cw(4).y, l_mino.rot(2).cw(5).x, l_mino.rot(2).cw(5).y, l_mino.rot(2).cw(6).x, l_mino.rot(2).cw(6).y, l_mino.rot(2).cw(7).x, l_mino.rot(2).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", l_mino.rot(3).cw(0).x, l_mino.rot(3).cw(0).y, l_mino.rot(3).cw(1).x, l_mino.rot(3).cw(1).y, l_mino.rot(3).cw(2).x, l_mino.rot(3).cw(2).y, l_mino.rot(3).cw(3).x, l_mino.rot(3).cw(3).y, l_mino.rot(3).cw(4).x, l_mino.rot(3).cw(4).y, l_mino.rot(3).cw(5).x, l_mino.rot(3).cw(5).y, l_mino.rot(3).cw(6).x, l_mino.rot(3).cw(6).y, l_mino.rot(3).cw(7).x, l_mino.rot(3).cw(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(0).cw(0).x, z_mino.rot(0).cw(0).y, z_mino.rot(0).cw(1).x, z_mino.rot(0).cw(1).y, z_mino.rot(0).cw(2).x, z_mino.rot(0).cw(2).y, z_mino.rot(0).cw(3).x, z_mino.rot(0).cw(3).y, z_mino.rot(0).cw(4).x, z_mino.rot(0).cw(4).y, z_mino.rot(0).cw(5).x, z_mino.rot(0).cw(5).y, z_mino.rot(0).cw(6).x, z_mino.rot(0).cw(6).y, z_mino.rot(0).cw(7).x, z_mino.rot(0).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(1).cw(0).x, z_mino.rot(1).cw(0).y, z_mino.rot(1).cw(1).x, z_mino.rot(1).cw(1).y, z_mino.rot(1).cw(2).x, z_mino.rot(1).cw(2).y, z_mino.rot(1).cw(3).x, z_mino.rot(1).cw(3).y, z_mino.rot(1).cw(4).x, z_mino.rot(1).cw(4).y, z_mino.rot(1).cw(5).x, z_mino.rot(1).cw(5).y, z_mino.rot(1).cw(6).x, z_mino.rot(1).cw(6).y, z_mino.rot(1).cw(7).x, z_mino.rot(1).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(2).cw(0).x, z_mino.rot(2).cw(0).y, z_mino.rot(2).cw(1).x, z_mino.rot(2).cw(1).y, z_mino.rot(2).cw(2).x, z_mino.rot(2).cw(2).y, z_mino.rot(2).cw(3).x, z_mino.rot(2).cw(3).y, z_mino.rot(2).cw(4).x, z_mino.rot(2).cw(4).y, z_mino.rot(2).cw(5).x, z_mino.rot(2).cw(5).y, z_mino.rot(2).cw(6).x, z_mino.rot(2).cw(6).y, z_mino.rot(2).cw(7).x, z_mino.rot(2).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", z_mino.rot(3).cw(0).x, z_mino.rot(3).cw(0).y, z_mino.rot(3).cw(1).x, z_mino.rot(3).cw(1).y, z_mino.rot(3).cw(2).x, z_mino.rot(3).cw(2).y, z_mino.rot(3).cw(3).x, z_mino.rot(3).cw(3).y, z_mino.rot(3).cw(4).x, z_mino.rot(3).cw(4).y, z_mino.rot(3).cw(5).x, z_mino.rot(3).cw(5).y, z_mino.rot(3).cw(6).x, z_mino.rot(3).cw(6).y, z_mino.rot(3).cw(7).x, z_mino.rot(3).cw(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", i_mino.rot(0).cw(0).x, i_mino.rot(0).cw(0).y, i_mino.rot(0).cw(1).x, i_mino.rot(0).cw(1).y, i_mino.rot(0).cw(2).x, i_mino.rot(0).cw(2).y, i_mino.rot(0).cw(3).x, i_mino.rot(0).cw(3).y, i_mino.rot(0).cw(4).x, i_mino.rot(0).cw(4).y, i_mino.rot(0).cw(5).x, i_mino.rot(0).cw(5).y, i_mino.rot(0).cw(6).x, i_mino.rot(0).cw(6).y, i_mino.rot(0).cw(7).x, i_mino.rot(0).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", i_mino.rot(1).cw(0).x, i_mino.rot(1).cw(0).y, i_mino.rot(1).cw(1).x, i_mino.rot(1).cw(1).y, i_mino.rot(1).cw(2).x, i_mino.rot(1).cw(2).y, i_mino.rot(1).cw(3).x, i_mino.rot(1).cw(3).y, i_mino.rot(1).cw(4).x, i_mino.rot(1).cw(4).y, i_mino.rot(1).cw(5).x, i_mino.rot(1).cw(5).y, i_mino.rot(1).cw(6).x, i_mino.rot(1).cw(6).y, i_mino.rot(1).cw(7).x, i_mino.rot(1).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", i_mino.rot(2).cw(0).x, i_mino.rot(2).cw(0).y, i_mino.rot(2).cw(1).x, i_mino.rot(2).cw(1).y, i_mino.rot(2).cw(2).x, i_mino.rot(2).cw(2).y, i_mino.rot(2).cw(3).x, i_mino.rot(2).cw(3).y, i_mino.rot(2).cw(4).x, i_mino.rot(2).cw(4).y, i_mino.rot(2).cw(5).x, i_mino.rot(2).cw(5).y, i_mino.rot(2).cw(6).x, i_mino.rot(2).cw(6).y, i_mino.rot(2).cw(7).x, i_mino.rot(2).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", i_mino.rot(3).cw(0).x, i_mino.rot(3).cw(0).y, i_mino.rot(3).cw(1).x, i_mino.rot(3).cw(1).y, i_mino.rot(3).cw(2).x, i_mino.rot(3).cw(2).y, i_mino.rot(3).cw(3).x, i_mino.rot(3).cw(3).y, i_mino.rot(3).cw(4).x, i_mino.rot(3).cw(4).y, i_mino.rot(3).cw(5).x, i_mino.rot(3).cw(5).y, i_mino.rot(3).cw(6).x, i_mino.rot(3).cw(6).y, i_mino.rot(3).cw(7).x, i_mino.rot(3).cw(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", t_mino.rot(0).cw(0).x, t_mino.rot(0).cw(0).y, t_mino.rot(0).cw(1).x, t_mino.rot(0).cw(1).y, t_mino.rot(0).cw(2).x, t_mino.rot(0).cw(2).y, t_mino.rot(0).cw(3).x, t_mino.rot(0).cw(3).y, t_mino.rot(0).cw(4).x, t_mino.rot(0).cw(4).y, t_mino.rot(0).cw(5).x, t_mino.rot(0).cw(5).y, t_mino.rot(0).cw(6).x, t_mino.rot(0).cw(6).y, t_mino.rot(0).cw(7).x, t_mino.rot(0).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", t_mino.rot(1).cw(0).x, t_mino.rot(1).cw(0).y, t_mino.rot(1).cw(1).x, t_mino.rot(1).cw(1).y, t_mino.rot(1).cw(2).x, t_mino.rot(1).cw(2).y, t_mino.rot(1).cw(3).x, t_mino.rot(1).cw(3).y, t_mino.rot(1).cw(4).x, t_mino.rot(1).cw(4).y, t_mino.rot(1).cw(5).x, t_mino.rot(1).cw(5).y, t_mino.rot(1).cw(6).x, t_mino.rot(1).cw(6).y, t_mino.rot(1).cw(7).x, t_mino.rot(1).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", t_mino.rot(2).cw(0).x, t_mino.rot(2).cw(0).y, t_mino.rot(2).cw(1).x, t_mino.rot(2).cw(1).y, t_mino.rot(2).cw(2).x, t_mino.rot(2).cw(2).y, t_mino.rot(2).cw(3).x, t_mino.rot(2).cw(3).y, t_mino.rot(2).cw(4).x, t_mino.rot(2).cw(4).y, t_mino.rot(2).cw(5).x, t_mino.rot(2).cw(5).y, t_mino.rot(2).cw(6).x, t_mino.rot(2).cw(6).y, t_mino.rot(2).cw(7).x, t_mino.rot(2).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", t_mino.rot(3).cw(0).x, t_mino.rot(3).cw(0).y, t_mino.rot(3).cw(1).x, t_mino.rot(3).cw(1).y, t_mino.rot(3).cw(2).x, t_mino.rot(3).cw(2).y, t_mino.rot(3).cw(3).x, t_mino.rot(3).cw(3).y, t_mino.rot(3).cw(4).x, t_mino.rot(3).cw(4).y, t_mino.rot(3).cw(5).x, t_mino.rot(3).cw(5).y, t_mino.rot(3).cw(6).x, t_mino.rot(3).cw(6).y, t_mino.rot(3).cw(7).x, t_mino.rot(3).cw(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", o_mino.rot(0).cw(0).x, o_mino.rot(0).cw(0).y, o_mino.rot(0).cw(1).x, o_mino.rot(0).cw(1).y, o_mino.rot(0).cw(2).x, o_mino.rot(0).cw(2).y, o_mino.rot(0).cw(3).x, o_mino.rot(0).cw(3).y, o_mino.rot(0).cw(4).x, o_mino.rot(0).cw(4).y, o_mino.rot(0).cw(5).x, o_mino.rot(0).cw(5).y, o_mino.rot(0).cw(6).x, o_mino.rot(0).cw(6).y, o_mino.rot(0).cw(7).x, o_mino.rot(0).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", o_mino.rot(1).cw(0).x, o_mino.rot(1).cw(0).y, o_mino.rot(1).cw(1).x, o_mino.rot(1).cw(1).y, o_mino.rot(1).cw(2).x, o_mino.rot(1).cw(2).y, o_mino.rot(1).cw(3).x, o_mino.rot(1).cw(3).y, o_mino.rot(1).cw(4).x, o_mino.rot(1).cw(4).y, o_mino.rot(1).cw(5).x, o_mino.rot(1).cw(5).y, o_mino.rot(1).cw(6).x, o_mino.rot(1).cw(6).y, o_mino.rot(1).cw(7).x, o_mino.rot(1).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", o_mino.rot(2).cw(0).x, o_mino.rot(2).cw(0).y, o_mino.rot(2).cw(1).x, o_mino.rot(2).cw(1).y, o_mino.rot(2).cw(2).x, o_mino.rot(2).cw(2).y, o_mino.rot(2).cw(3).x, o_mino.rot(2).cw(3).y, o_mino.rot(2).cw(4).x, o_mino.rot(2).cw(4).y, o_mino.rot(2).cw(5).x, o_mino.rot(2).cw(5).y, o_mino.rot(2).cw(6).x, o_mino.rot(2).cw(6).y, o_mino.rot(2).cw(7).x, o_mino.rot(2).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", o_mino.rot(3).cw(0).x, o_mino.rot(3).cw(0).y, o_mino.rot(3).cw(1).x, o_mino.rot(3).cw(1).y, o_mino.rot(3).cw(2).x, o_mino.rot(3).cw(2).y, o_mino.rot(3).cw(3).x, o_mino.rot(3).cw(3).y, o_mino.rot(3).cw(4).x, o_mino.rot(3).cw(4).y, o_mino.rot(3).cw(5).x, o_mino.rot(3).cw(5).y, o_mino.rot(3).cw(6).x, o_mino.rot(3).cw(6).y, o_mino.rot(3).cw(7).x, o_mino.rot(3).cw(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(0).cw(0).x, z_mino.rot(0).cw(0).y, z_mino.rot(0).cw(1).x, z_mino.rot(0).cw(1).y, z_mino.rot(0).cw(2).x, z_mino.rot(0).cw(2).y, z_mino.rot(0).cw(3).x, z_mino.rot(0).cw(3).y, z_mino.rot(0).cw(4).x, z_mino.rot(0).cw(4).y, z_mino.rot(0).cw(5).x, z_mino.rot(0).cw(5).y, z_mino.rot(0).cw(6).x, z_mino.rot(0).cw(6).y, z_mino.rot(0).cw(7).x, z_mino.rot(0).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(1).cw(0).x, z_mino.rot(1).cw(0).y, z_mino.rot(1).cw(1).x, z_mino.rot(1).cw(1).y, z_mino.rot(1).cw(2).x, z_mino.rot(1).cw(2).y, z_mino.rot(1).cw(3).x, z_mino.rot(1).cw(3).y, z_mino.rot(1).cw(4).x, z_mino.rot(1).cw(4).y, z_mino.rot(1).cw(5).x, z_mino.rot(1).cw(5).y, z_mino.rot(1).cw(6).x, z_mino.rot(1).cw(6).y, z_mino.rot(1).cw(7).x, z_mino.rot(1).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(2).cw(0).x, z_mino.rot(2).cw(0).y, z_mino.rot(2).cw(1).x, z_mino.rot(2).cw(1).y, z_mino.rot(2).cw(2).x, z_mino.rot(2).cw(2).y, z_mino.rot(2).cw(3).x, z_mino.rot(2).cw(3).y, z_mino.rot(2).cw(4).x, z_mino.rot(2).cw(4).y, z_mino.rot(2).cw(5).x, z_mino.rot(2).cw(5).y, z_mino.rot(2).cw(6).x, z_mino.rot(2).cw(6).y, z_mino.rot(2).cw(7).x, z_mino.rot(2).cw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", z_mino.rot(3).cw(0).x, z_mino.rot(3).cw(0).y, z_mino.rot(3).cw(1).x, z_mino.rot(3).cw(1).y, z_mino.rot(3).cw(2).x, z_mino.rot(3).cw(2).y, z_mino.rot(3).cw(3).x, z_mino.rot(3).cw(3).y, z_mino.rot(3).cw(4).x, z_mino.rot(3).cw(4).y, z_mino.rot(3).cw(5).x, z_mino.rot(3).cw(5).y, z_mino.rot(3).cw(6).x, z_mino.rot(3).cw(6).y, z_mino.rot(3).cw(7).x, z_mino.rot(3).cw(7).y);
	fprintf(out, "	};\n");
	fprintf(out, "	constexpr Minos::Coord ccw_offset[7][4][8] = {\n");
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", s_mino.rot(0).ccw(0).x, s_mino.rot(0).ccw(0).y, s_mino.rot(0).ccw(1).x, s_mino.rot(0).ccw(1).y, s_mino.rot(0).ccw(2).x, s_mino.rot(0).ccw(2).y, s_mino.rot(0).ccw(3).x, s_mino.rot(0).ccw(3).y, s_mino.rot(0).ccw(4).x, s_mino.rot(0).ccw(4).y, s_mino.rot(0).ccw(5).x, s_mino.rot(0).ccw(5).y, s_mino.rot(0).ccw(6).x, s_mino.rot(0).ccw(6).y, s_mino.rot(0).ccw(7).x, s_mino.rot(0).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", s_mino.rot(1).ccw(0).x, s_mino.rot(1).ccw(0).y, s_mino.rot(1).ccw(1).x, s_mino.rot(1).ccw(1).y, s_mino.rot(1).ccw(2).x, s_mino.rot(1).ccw(2).y, s_mino.rot(1).ccw(3).x, s_mino.rot(1).ccw(3).y, s_mino.rot(1).ccw(4).x, s_mino.rot(1).ccw(4).y, s_mino.rot(1).ccw(5).x, s_mino.rot(1).ccw(5).y, s_mino.rot(1).ccw(6).x, s_mino.rot(1).ccw(6).y, s_mino.rot(1).ccw(7).x, s_mino.rot(1).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", s_mino.rot(2).ccw(0).x, s_mino.rot(2).ccw(0).y, s_mino.rot(2).ccw(1).x, s_mino.rot(2).ccw(1).y, s_mino.rot(2).ccw(2).x, s_mino.rot(2).ccw(2).y, s_mino.rot(2).ccw(3).x, s_mino.rot(2).ccw(3).y, s_mino.rot(2).ccw(4).x, s_mino.rot(2).ccw(4).y, s_mino.rot(2).ccw(5).x, s_mino.rot(2).ccw(5).y, s_mino.rot(2).ccw(6).x, s_mino.rot(2).ccw(6).y, s_mino.rot(2).ccw(7).x, s_mino.rot(2).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", s_mino.rot(3).ccw(0).x, s_mino.rot(3).ccw(0).y, s_mino.rot(3).ccw(1).x, s_mino.rot(3).ccw(1).y, s_mino.rot(3).ccw(2).x, s_mino.rot(3).ccw(2).y, s_mino.rot(3).ccw(3).x, s_mino.rot(3).ccw(3).y, s_mino.rot(3).ccw(4).x, s_mino.rot(3).ccw(4).y, s_mino.rot(3).ccw(5).x, s_mino.rot(3).ccw(5).y, s_mino.rot(3).ccw(6).x, s_mino.rot(3).ccw(6).y, s_mino.rot(3).ccw(7).x, s_mino.rot(3).ccw(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", l_mino.rot(0).ccw(0).x, l_mino.rot(0).ccw(0).y, l_mino.rot(0).ccw(1).x, l_mino.rot(0).ccw(1).y, l_mino.rot(0).ccw(2).x, l_mino.rot(0).ccw(2).y, l_mino.rot(0).ccw(3).x, l_mino.rot(0).ccw(3).y, l_mino.rot(0).ccw(4).x, l_mino.rot(0).ccw(4).y, l_mino.rot(0).ccw(5).x, l_mino.rot(0).ccw(5).y, l_mino.rot(0).ccw(6).x, l_mino.rot(0).ccw(6).y, l_mino.rot(0).ccw(7).x, l_mino.rot(0).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", l_mino.rot(1).ccw(0).x, l_mino.rot(1).ccw(0).y, l_mino.rot(1).ccw(1).x, l_mino.rot(1).ccw(1).y, l_mino.rot(1).ccw(2).x, l_mino.rot(1).ccw(2).y, l_mino.rot(1).ccw(3).x, l_mino.rot(1).ccw(3).y, l_mino.rot(1).ccw(4).x, l_mino.rot(1).ccw(4).y, l_mino.rot(1).ccw(5).x, l_mino.rot(1).ccw(5).y, l_mino.rot(1).ccw(6).x, l_mino.rot(1).ccw(6).y, l_mino.rot(1).ccw(7).x, l_mino.rot(1).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", l_mino.rot(2).ccw(0).x, l_mino.rot(2).ccw(0).y, l_mino.rot(2).ccw(1).x, l_mino.rot(2).ccw(1).y, l_mino.rot(2).ccw(2).x, l_mino.rot(2).ccw(2).y, l_mino.rot(2).ccw(3).x, l_mino.rot(2).ccw(3).y, l_mino.rot(2).ccw(4).x, l_mino.rot(2).ccw(4).y, l_mino.rot(2).ccw(5).x, l_mino.rot(2).ccw(5).y, l_mino.rot(2).ccw(6).x, l_mino.rot(2).ccw(6).y, l_mino.rot(2).ccw(7).x, l_mino.rot(2).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", l_mino.rot(3).ccw(0).x, l_mino.rot(3).ccw(0).y, l_mino.rot(3).ccw(1).x, l_mino.rot(3).ccw(1).y, l_mino.rot(3).ccw(2).x, l_mino.rot(3).ccw(2).y, l_mino.rot(3).ccw(3).x, l_mino.rot(3).ccw(3).y, l_mino.rot(3).ccw(4).x, l_mino.rot(3).ccw(4).y, l_mino.rot(3).ccw(5).x, l_mino.rot(3).ccw(5).y, l_mino.rot(3).ccw(6).x, l_mino.rot(3).ccw(6).y, l_mino.rot(3).ccw(7).x, l_mino.rot(3).ccw(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(0).ccw(0).x, z_mino.rot(0).ccw(0).y, z_mino.rot(0).ccw(1).x, z_mino.rot(0).ccw(1).y, z_mino.rot(0).ccw(2).x, z_mino.rot(0).ccw(2).y, z_mino.rot(0).ccw(3).x, z_mino.rot(0).ccw(3).y, z_mino.rot(0).ccw(4).x, z_mino.rot(0).ccw(4).y, z_mino.rot(0).ccw(5).x, z_mino.rot(0).ccw(5).y, z_mino.rot(0).ccw(6).x, z_mino.rot(0).ccw(6).y, z_mino.rot(0).ccw(7).x, z_mino.rot(0).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(1).ccw(0).x, z_mino.rot(1).ccw(0).y, z_mino.rot(1).ccw(1).x, z_mino.rot(1).ccw(1).y, z_mino.rot(1).ccw(2).x, z_mino.rot(1).ccw(2).y, z_mino.rot(1).ccw(3).x, z_mino.rot(1).ccw(3).y, z_mino.rot(1).ccw(4).x, z_mino.rot(1).ccw(4).y, z_mino.rot(1).ccw(5).x, z_mino.rot(1).ccw(5).y, z_mino.rot(1).ccw(6).x, z_mino.rot(1).ccw(6).y, z_mino.rot(1).ccw(7).x, z_mino.rot(1).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(2).ccw(0).x, z_mino.rot(2).ccw(0).y, z_mino.rot(2).ccw(1).x, z_mino.rot(2).ccw(1).y, z_mino.rot(2).ccw(2).x, z_mino.rot(2).ccw(2).y, z_mino.rot(2).ccw(3).x, z_mino.rot(2).ccw(3).y, z_mino.rot(2).ccw(4).x, z_mino.rot(2).ccw(4).y, z_mino.rot(2).ccw(5).x, z_mino.rot(2).ccw(5).y, z_mino.rot(2).ccw(6).x, z_mino.rot(2).ccw(6).y, z_mino.rot(2).ccw(7).x, z_mino.rot(2).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", z_mino.rot(3).ccw(0).x, z_mino.rot(3).ccw(0).y, z_mino.rot(3).ccw(1).x, z_mino.rot(3).ccw(1).y, z_mino.rot(3).ccw(2).x, z_mino.rot(3).ccw(2).y, z_mino.rot(3).ccw(3).x, z_mino.rot(3).ccw(3).y, z_mino.rot(3).ccw(4).x, z_mino.rot(3).ccw(4).y, z_mino.rot(3).ccw(5).x, z_mino.rot(3).ccw(5).y, z_mino.rot(3).ccw(6).x, z_mino.rot(3).ccw(6).y, z_mino.rot(3).ccw(7).x, z_mino.rot(3).ccw(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", i_mino.rot(0).ccw(0).x, i_mino.rot(0).ccw(0).y, i_mino.rot(0).ccw(1).x, i_mino.rot(0).ccw(1).y, i_mino.rot(0).ccw(2).x, i_mino.rot(0).ccw(2).y, i_mino.rot(0).ccw(3).x, i_mino.rot(0).ccw(3).y, i_mino.rot(0).ccw(4).x, i_mino.rot(0).ccw(4).y, i_mino.rot(0).ccw(5).x, i_mino.rot(0).ccw(5).y, i_mino.rot(0).ccw(6).x, i_mino.rot(0).ccw(6).y, i_mino.rot(0).ccw(7).x, i_mino.rot(0).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", i_mino.rot(1).ccw(0).x, i_mino.rot(1).ccw(0).y, i_mino.rot(1).ccw(1).x, i_mino.rot(1).ccw(1).y, i_mino.rot(1).ccw(2).x, i_mino.rot(1).ccw(2).y, i_mino.rot(1).ccw(3).x, i_mino.rot(1).ccw(3).y, i_mino.rot(1).ccw(4).x, i_mino.rot(1).ccw(4).y, i_mino.rot(1).ccw(5).x, i_mino.rot(1).ccw(5).y, i_mino.rot(1).ccw(6).x, i_mino.rot(1).ccw(6).y, i_mino.rot(1).ccw(7).x, i_mino.rot(1).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", i_mino.rot(2).ccw(0).x, i_mino.rot(2).ccw(0).y, i_mino.rot(2).ccw(1).x, i_mino.rot(2).ccw(1).y, i_mino.rot(2).ccw(2).x, i_mino.rot(2).ccw(2).y, i_mino.rot(2).ccw(3).x, i_mino.rot(2).ccw(3).y, i_mino.rot(2).ccw(4).x, i_mino.rot(2).ccw(4).y, i_mino.rot(2).ccw(5).x, i_mino.rot(2).ccw(5).y, i_mino.rot(2).ccw(6).x, i_mino.rot(2).ccw(6).y, i_mino.rot(2).ccw(7).x, i_mino.rot(2).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", i_mino.rot(3).ccw(0).x, i_mino.rot(3).ccw(0).y, i_mino.rot(3).ccw(1).x, i_mino.rot(3).ccw(1).y, i_mino.rot(3).ccw(2).x, i_mino.rot(3).ccw(2).y, i_mino.rot(3).ccw(3).x, i_mino.rot(3).ccw(3).y, i_mino.rot(3).ccw(4).x, i_mino.rot(3).ccw(4).y, i_mino.rot(3).ccw(5).x, i_mino.rot(3).ccw(5).y, i_mino.rot(3).ccw(6).x, i_mino.rot(3).ccw(6).y, i_mino.rot(3).ccw(7).x, i_mino.rot(3).ccw(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", t_mino.rot(0).ccw(0).x, t_mino.rot(0).ccw(0).y, t_mino.rot(0).ccw(1).x, t_mino.rot(0).ccw(1).y, t_mino.rot(0).ccw(2).x, t_mino.rot(0).ccw(2).y, t_mino.rot(0).ccw(3).x, t_mino.rot(0).ccw(3).y, t_mino.rot(0).ccw(4).x, t_mino.rot(0).ccw(4).y, t_mino.rot(0).ccw(5).x, t_mino.rot(0).ccw(5).y, t_mino.rot(0).ccw(6).x, t_mino.rot(0).ccw(6).y, t_mino.rot(0).ccw(7).x, t_mino.rot(0).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", t_mino.rot(1).ccw(0).x, t_mino.rot(1).ccw(0).y, t_mino.rot(1).ccw(1).x, t_mino.rot(1).ccw(1).y, t_mino.rot(1).ccw(2).x, t_mino.rot(1).ccw(2).y, t_mino.rot(1).ccw(3).x, t_mino.rot(1).ccw(3).y, t_mino.rot(1).ccw(4).x, t_mino.rot(1).ccw(4).y, t_mino.rot(1).ccw(5).x, t_mino.rot(1).ccw(5).y, t_mino.rot(1).ccw(6).x, t_mino.rot(1).ccw(6).y, t_mino.rot(1).ccw(7).x, t_mino.rot(1).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", t_mino.rot(2).ccw(0).x, t_mino.rot(2).ccw(0).y, t_mino.rot(2).ccw(1).x, t_mino.rot(2).ccw(1).y, t_mino.rot(2).ccw(2).x, t_mino.rot(2).ccw(2).y, t_mino.rot(2).ccw(3).x, t_mino.rot(2).ccw(3).y, t_mino.rot(2).ccw(4).x, t_mino.rot(2).ccw(4).y, t_mino.rot(2).ccw(5).x, t_mino.rot(2).ccw(5).y, t_mino.rot(2).ccw(6).x, t_mino.rot(2).ccw(6).y, t_mino.rot(2).ccw(7).x, t_mino.rot(2).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", t_mino.rot(3).ccw(0).x, t_mino.rot(3).ccw(0).y, t_mino.rot(3).ccw(1).x, t_mino.rot(3).ccw(1).y, t_mino.rot(3).ccw(2).x, t_mino.rot(3).ccw(2).y, t_mino.rot(3).ccw(3).x, t_mino.rot(3).ccw(3).y, t_mino.rot(3).ccw(4).x, t_mino.rot(3).ccw(4).y, t_mino.rot(3).ccw(5).x, t_mino.rot(3).ccw(5).y, t_mino.rot(3).ccw(6).x, t_mino.rot(3).ccw(6).y, t_mino.rot(3).ccw(7).x, t_mino.rot(3).ccw(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", o_mino.rot(0).ccw(0).x, o_mino.rot(0).ccw(0).y, o_mino.rot(0).ccw(1).x, o_mino.rot(0).ccw(1).y, o_mino.rot(0).ccw(2).x, o_mino.rot(0).ccw(2).y, o_mino.rot(0).ccw(3).x, o_mino.rot(0).ccw(3).y, o_mino.rot(0).ccw(4).x, o_mino.rot(0).ccw(4).y, o_mino.rot(0).ccw(5).x, o_mino.rot(0).ccw(5).y, o_mino.rot(0).ccw(6).x, o_mino.rot(0).ccw(6).y, o_mino.rot(0).ccw(7).x, o_mino.rot(0).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", o_mino.rot(1).ccw(0).x, o_mino.rot(1).ccw(0).y, o_mino.rot(1).ccw(1).x, o_mino.rot(1).ccw(1).y, o_mino.rot(1).ccw(2).x, o_mino.rot(1).ccw(2).y, o_mino.rot(1).ccw(3).x, o_mino.rot(1).ccw(3).y, o_mino.rot(1).ccw(4).x, o_mino.rot(1).ccw(4).y, o_mino.rot(1).ccw(5).x, o_mino.rot(1).ccw(5).y, o_mino.rot(1).ccw(6).x, o_mino.rot(1).ccw(6).y, o_mino.rot(1).ccw(7).x, o_mino.rot(1).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", o_mino.rot(2).ccw(0).x, o_mino.rot(2).ccw(0).y, o_mino.rot(2).ccw(1).x, o_mino.rot(2).ccw(1).y, o_mino.rot(2).ccw(2).x, o_mino.rot(2).ccw(2).y, o_mino.rot(2).ccw(3).x, o_mino.rot(2).ccw(3).y, o_mino.rot(2).ccw(4).x, o_mino.rot(2).ccw(4).y, o_mino.rot(2).ccw(5).x, o_mino.rot(2).ccw(5).y, o_mino.rot(2).ccw(6).x, o_mino.rot(2).ccw(6).y, o_mino.rot(2).ccw(7).x, o_mino.rot(2).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", o_mino.rot(3).ccw(0).x, o_mino.rot(3).ccw(0).y, o_mino.rot(3).ccw(1).x, o_mino.rot(3).ccw(1).y, o_mino.rot(3).ccw(2).x, o_mino.rot(3).ccw(2).y, o_mino.rot(3).ccw(3).x, o_mino.rot(3).ccw(3).y, o_mino.rot(3).ccw(4).x, o_mino.rot(3).ccw(4).y, o_mino.rot(3).ccw(5).x, o_mino.rot(3).ccw(5).y, o_mino.rot(3).ccw(6).x, o_mino.rot(3).ccw(6).y, o_mino.rot(3).ccw(7).x, o_mino.rot(3).ccw(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(0).ccw(0).x, z_mino.rot(0).ccw(0).y, z_mino.rot(0).ccw(1).x, z_mino.rot(0).ccw(1).y, z_mino.rot(0).ccw(2).x, z_mino.rot(0).ccw(2).y, z_mino.rot(0).ccw(3).x, z_mino.rot(0).ccw(3).y, z_mino.rot(0).ccw(4).x, z_mino.rot(0).ccw(4).y, z_mino.rot(0).ccw(5).x, z_mino.rot(0).ccw(5).y, z_mino.rot(0).ccw(6).x, z_mino.rot(0).ccw(6).y, z_mino.rot(0).ccw(7).x, z_mino.rot(0).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(1).ccw(0).x, z_mino.rot(1).ccw(0).y, z_mino.rot(1).ccw(1).x, z_mino.rot(1).ccw(1).y, z_mino.rot(1).ccw(2).x, z_mino.rot(1).ccw(2).y, z_mino.rot(1).ccw(3).x, z_mino.rot(1).ccw(3).y, z_mino.rot(1).ccw(4).x, z_mino.rot(1).ccw(4).y, z_mino.rot(1).ccw(5).x, z_mino.rot(1).ccw(5).y, z_mino.rot(1).ccw(6).x, z_mino.rot(1).ccw(6).y, z_mino.rot(1).ccw(7).x, z_mino.rot(1).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(2).ccw(0).x, z_mino.rot(2).ccw(0).y, z_mino.rot(2).ccw(1).x, z_mino.rot(2).ccw(1).y, z_mino.rot(2).ccw(2).x, z_mino.rot(2).ccw(2).y, z_mino.rot(2).ccw(3).x, z_mino.rot(2).ccw(3).y, z_mino.rot(2).ccw(4).x, z_mino.rot(2).ccw(4).y, z_mino.rot(2).ccw(5).x, z_mino.rot(2).ccw(5).y, z_mino.rot(2).ccw(6).x, z_mino.rot(2).ccw(6).y, z_mino.rot(2).ccw(7).x, z_mino.rot(2).ccw(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", z_mino.rot(3).ccw(0).x, z_mino.rot(3).ccw(0).y, z_mino.rot(3).ccw(1).x, z_mino.rot(3).ccw(1).y, z_mino.rot(3).ccw(2).x, z_mino.rot(3).ccw(2).y, z_mino.rot(3).ccw(3).x, z_mino.rot(3).ccw(3).y, z_mino.rot(3).ccw(4).x, z_mino.rot(3).ccw(4).y, z_mino.rot(3).ccw(5).x, z_mino.rot(3).ccw(5).y, z_mino.rot(3).ccw(6).x, z_mino.rot(3).ccw(6).y, z_mino.rot(3).ccw(7).x, z_mino.rot(3).ccw(7).y);
	fprintf(out, "	};\n");
	fprintf(out, "	constexpr Minos::Coord _180_offset[7][4][8] = {\n");
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", s_mino.rot(0)._180(0).x, s_mino.rot(0)._180(0).y, s_mino.rot(0)._180(1).x, s_mino.rot(0)._180(1).y, s_mino.rot(0)._180(2).x, s_mino.rot(0)._180(2).y, s_mino.rot(0)._180(3).x, s_mino.rot(0)._180(3).y, s_mino.rot(0)._180(4).x, s_mino.rot(0)._180(4).y, s_mino.rot(0)._180(5).x, s_mino.rot(0)._180(5).y, s_mino.rot(0)._180(6).x, s_mino.rot(0)._180(6).y, s_mino.rot(0)._180(7).x, s_mino.rot(0)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", s_mino.rot(1)._180(0).x, s_mino.rot(1)._180(0).y, s_mino.rot(1)._180(1).x, s_mino.rot(1)._180(1).y, s_mino.rot(1)._180(2).x, s_mino.rot(1)._180(2).y, s_mino.rot(1)._180(3).x, s_mino.rot(1)._180(3).y, s_mino.rot(1)._180(4).x, s_mino.rot(1)._180(4).y, s_mino.rot(1)._180(5).x, s_mino.rot(1)._180(5).y, s_mino.rot(1)._180(6).x, s_mino.rot(1)._180(6).y, s_mino.rot(1)._180(7).x, s_mino.rot(1)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", s_mino.rot(2)._180(0).x, s_mino.rot(2)._180(0).y, s_mino.rot(2)._180(1).x, s_mino.rot(2)._180(1).y, s_mino.rot(2)._180(2).x, s_mino.rot(2)._180(2).y, s_mino.rot(2)._180(3).x, s_mino.rot(2)._180(3).y, s_mino.rot(2)._180(4).x, s_mino.rot(2)._180(4).y, s_mino.rot(2)._180(5).x, s_mino.rot(2)._180(5).y, s_mino.rot(2)._180(6).x, s_mino.rot(2)._180(6).y, s_mino.rot(2)._180(7).x, s_mino.rot(2)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", s_mino.rot(3)._180(0).x, s_mino.rot(3)._180(0).y, s_mino.rot(3)._180(1).x, s_mino.rot(3)._180(1).y, s_mino.rot(3)._180(2).x, s_mino.rot(3)._180(2).y, s_mino.rot(3)._180(3).x, s_mino.rot(3)._180(3).y, s_mino.rot(3)._180(4).x, s_mino.rot(3)._180(4).y, s_mino.rot(3)._180(5).x, s_mino.rot(3)._180(5).y, s_mino.rot(3)._180(6).x, s_mino.rot(3)._180(6).y, s_mino.rot(3)._180(7).x, s_mino.rot(3)._180(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", l_mino.rot(0)._180(0).x, l_mino.rot(0)._180(0).y, l_mino.rot(0)._180(1).x, l_mino.rot(0)._180(1).y, l_mino.rot(0)._180(2).x, l_mino.rot(0)._180(2).y, l_mino.rot(0)._180(3).x, l_mino.rot(0)._180(3).y, l_mino.rot(0)._180(4).x, l_mino.rot(0)._180(4).y, l_mino.rot(0)._180(5).x, l_mino.rot(0)._180(5).y, l_mino.rot(0)._180(6).x, l_mino.rot(0)._180(6).y, l_mino.rot(0)._180(7).x, l_mino.rot(0)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", l_mino.rot(1)._180(0).x, l_mino.rot(1)._180(0).y, l_mino.rot(1)._180(1).x, l_mino.rot(1)._180(1).y, l_mino.rot(1)._180(2).x, l_mino.rot(1)._180(2).y, l_mino.rot(1)._180(3).x, l_mino.rot(1)._180(3).y, l_mino.rot(1)._180(4).x, l_mino.rot(1)._180(4).y, l_mino.rot(1)._180(5).x, l_mino.rot(1)._180(5).y, l_mino.rot(1)._180(6).x, l_mino.rot(1)._180(6).y, l_mino.rot(1)._180(7).x, l_mino.rot(1)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", l_mino.rot(2)._180(0).x, l_mino.rot(2)._180(0).y, l_mino.rot(2)._180(1).x, l_mino.rot(2)._180(1).y, l_mino.rot(2)._180(2).x, l_mino.rot(2)._180(2).y, l_mino.rot(2)._180(3).x, l_mino.rot(2)._180(3).y, l_mino.rot(2)._180(4).x, l_mino.rot(2)._180(4).y, l_mino.rot(2)._180(5).x, l_mino.rot(2)._180(5).y, l_mino.rot(2)._180(6).x, l_mino.rot(2)._180(6).y, l_mino.rot(2)._180(7).x, l_mino.rot(2)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", l_mino.rot(3)._180(0).x, l_mino.rot(3)._180(0).y, l_mino.rot(3)._180(1).x, l_mino.rot(3)._180(1).y, l_mino.rot(3)._180(2).x, l_mino.rot(3)._180(2).y, l_mino.rot(3)._180(3).x, l_mino.rot(3)._180(3).y, l_mino.rot(3)._180(4).x, l_mino.rot(3)._180(4).y, l_mino.rot(3)._180(5).x, l_mino.rot(3)._180(5).y, l_mino.rot(3)._180(6).x, l_mino.rot(3)._180(6).y, l_mino.rot(3)._180(7).x, l_mino.rot(3)._180(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(0)._180(0).x, z_mino.rot(0)._180(0).y, z_mino.rot(0)._180(1).x, z_mino.rot(0)._180(1).y, z_mino.rot(0)._180(2).x, z_mino.rot(0)._180(2).y, z_mino.rot(0)._180(3).x, z_mino.rot(0)._180(3).y, z_mino.rot(0)._180(4).x, z_mino.rot(0)._180(4).y, z_mino.rot(0)._180(5).x, z_mino.rot(0)._180(5).y, z_mino.rot(0)._180(6).x, z_mino.rot(0)._180(6).y, z_mino.rot(0)._180(7).x, z_mino.rot(0)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(1)._180(0).x, z_mino.rot(1)._180(0).y, z_mino.rot(1)._180(1).x, z_mino.rot(1)._180(1).y, z_mino.rot(1)._180(2).x, z_mino.rot(1)._180(2).y, z_mino.rot(1)._180(3).x, z_mino.rot(1)._180(3).y, z_mino.rot(1)._180(4).x, z_mino.rot(1)._180(4).y, z_mino.rot(1)._180(5).x, z_mino.rot(1)._180(5).y, z_mino.rot(1)._180(6).x, z_mino.rot(1)._180(6).y, z_mino.rot(1)._180(7).x, z_mino.rot(1)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(2)._180(0).x, z_mino.rot(2)._180(0).y, z_mino.rot(2)._180(1).x, z_mino.rot(2)._180(1).y, z_mino.rot(2)._180(2).x, z_mino.rot(2)._180(2).y, z_mino.rot(2)._180(3).x, z_mino.rot(2)._180(3).y, z_mino.rot(2)._180(4).x, z_mino.rot(2)._180(4).y, z_mino.rot(2)._180(5).x, z_mino.rot(2)._180(5).y, z_mino.rot(2)._180(6).x, z_mino.rot(2)._180(6).y, z_mino.rot(2)._180(7).x, z_mino.rot(2)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", z_mino.rot(3)._180(0).x, z_mino.rot(3)._180(0).y, z_mino.rot(3)._180(1).x, z_mino.rot(3)._180(1).y, z_mino.rot(3)._180(2).x, z_mino.rot(3)._180(2).y, z_mino.rot(3)._180(3).x, z_mino.rot(3)._180(3).y, z_mino.rot(3)._180(4).x, z_mino.rot(3)._180(4).y, z_mino.rot(3)._180(5).x, z_mino.rot(3)._180(5).y, z_mino.rot(3)._180(6).x, z_mino.rot(3)._180(6).y, z_mino.rot(3)._180(7).x, z_mino.rot(3)._180(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", i_mino.rot(0)._180(0).x, i_mino.rot(0)._180(0).y, i_mino.rot(0)._180(1).x, i_mino.rot(0)._180(1).y, i_mino.rot(0)._180(2).x, i_mino.rot(0)._180(2).y, i_mino.rot(0)._180(3).x, i_mino.rot(0)._180(3).y, i_mino.rot(0)._180(4).x, i_mino.rot(0)._180(4).y, i_mino.rot(0)._180(5).x, i_mino.rot(0)._180(5).y, i_mino.rot(0)._180(6).x, i_mino.rot(0)._180(6).y, i_mino.rot(0)._180(7).x, i_mino.rot(0)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", i_mino.rot(1)._180(0).x, i_mino.rot(1)._180(0).y, i_mino.rot(1)._180(1).x, i_mino.rot(1)._180(1).y, i_mino.rot(1)._180(2).x, i_mino.rot(1)._180(2).y, i_mino.rot(1)._180(3).x, i_mino.rot(1)._180(3).y, i_mino.rot(1)._180(4).x, i_mino.rot(1)._180(4).y, i_mino.rot(1)._180(5).x, i_mino.rot(1)._180(5).y, i_mino.rot(1)._180(6).x, i_mino.rot(1)._180(6).y, i_mino.rot(1)._180(7).x, i_mino.rot(1)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", i_mino.rot(2)._180(0).x, i_mino.rot(2)._180(0).y, i_mino.rot(2)._180(1).x, i_mino.rot(2)._180(1).y, i_mino.rot(2)._180(2).x, i_mino.rot(2)._180(2).y, i_mino.rot(2)._180(3).x, i_mino.rot(2)._180(3).y, i_mino.rot(2)._180(4).x, i_mino.rot(2)._180(4).y, i_mino.rot(2)._180(5).x, i_mino.rot(2)._180(5).y, i_mino.rot(2)._180(6).x, i_mino.rot(2)._180(6).y, i_mino.rot(2)._180(7).x, i_mino.rot(2)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", i_mino.rot(3)._180(0).x, i_mino.rot(3)._180(0).y, i_mino.rot(3)._180(1).x, i_mino.rot(3)._180(1).y, i_mino.rot(3)._180(2).x, i_mino.rot(3)._180(2).y, i_mino.rot(3)._180(3).x, i_mino.rot(3)._180(3).y, i_mino.rot(3)._180(4).x, i_mino.rot(3)._180(4).y, i_mino.rot(3)._180(5).x, i_mino.rot(3)._180(5).y, i_mino.rot(3)._180(6).x, i_mino.rot(3)._180(6).y, i_mino.rot(3)._180(7).x, i_mino.rot(3)._180(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", t_mino.rot(0)._180(0).x, t_mino.rot(0)._180(0).y, t_mino.rot(0)._180(1).x, t_mino.rot(0)._180(1).y, t_mino.rot(0)._180(2).x, t_mino.rot(0)._180(2).y, t_mino.rot(0)._180(3).x, t_mino.rot(0)._180(3).y, t_mino.rot(0)._180(4).x, t_mino.rot(0)._180(4).y, t_mino.rot(0)._180(5).x, t_mino.rot(0)._180(5).y, t_mino.rot(0)._180(6).x, t_mino.rot(0)._180(6).y, t_mino.rot(0)._180(7).x, t_mino.rot(0)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", t_mino.rot(1)._180(0).x, t_mino.rot(1)._180(0).y, t_mino.rot(1)._180(1).x, t_mino.rot(1)._180(1).y, t_mino.rot(1)._180(2).x, t_mino.rot(1)._180(2).y, t_mino.rot(1)._180(3).x, t_mino.rot(1)._180(3).y, t_mino.rot(1)._180(4).x, t_mino.rot(1)._180(4).y, t_mino.rot(1)._180(5).x, t_mino.rot(1)._180(5).y, t_mino.rot(1)._180(6).x, t_mino.rot(1)._180(6).y, t_mino.rot(1)._180(7).x, t_mino.rot(1)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", t_mino.rot(2)._180(0).x, t_mino.rot(2)._180(0).y, t_mino.rot(2)._180(1).x, t_mino.rot(2)._180(1).y, t_mino.rot(2)._180(2).x, t_mino.rot(2)._180(2).y, t_mino.rot(2)._180(3).x, t_mino.rot(2)._180(3).y, t_mino.rot(2)._180(4).x, t_mino.rot(2)._180(4).y, t_mino.rot(2)._180(5).x, t_mino.rot(2)._180(5).y, t_mino.rot(2)._180(6).x, t_mino.rot(2)._180(6).y, t_mino.rot(2)._180(7).x, t_mino.rot(2)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", t_mino.rot(3)._180(0).x, t_mino.rot(3)._180(0).y, t_mino.rot(3)._180(1).x, t_mino.rot(3)._180(1).y, t_mino.rot(3)._180(2).x, t_mino.rot(3)._180(2).y, t_mino.rot(3)._180(3).x, t_mino.rot(3)._180(3).y, t_mino.rot(3)._180(4).x, t_mino.rot(3)._180(4).y, t_mino.rot(3)._180(5).x, t_mino.rot(3)._180(5).y, t_mino.rot(3)._180(6).x, t_mino.rot(3)._180(6).y, t_mino.rot(3)._180(7).x, t_mino.rot(3)._180(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", o_mino.rot(0)._180(0).x, o_mino.rot(0)._180(0).y, o_mino.rot(0)._180(1).x, o_mino.rot(0)._180(1).y, o_mino.rot(0)._180(2).x, o_mino.rot(0)._180(2).y, o_mino.rot(0)._180(3).x, o_mino.rot(0)._180(3).y, o_mino.rot(0)._180(4).x, o_mino.rot(0)._180(4).y, o_mino.rot(0)._180(5).x, o_mino.rot(0)._180(5).y, o_mino.rot(0)._180(6).x, o_mino.rot(0)._180(6).y, o_mino.rot(0)._180(7).x, o_mino.rot(0)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", o_mino.rot(1)._180(0).x, o_mino.rot(1)._180(0).y, o_mino.rot(1)._180(1).x, o_mino.rot(1)._180(1).y, o_mino.rot(1)._180(2).x, o_mino.rot(1)._180(2).y, o_mino.rot(1)._180(3).x, o_mino.rot(1)._180(3).y, o_mino.rot(1)._180(4).x, o_mino.rot(1)._180(4).y, o_mino.rot(1)._180(5).x, o_mino.rot(1)._180(5).y, o_mino.rot(1)._180(6).x, o_mino.rot(1)._180(6).y, o_mino.rot(1)._180(7).x, o_mino.rot(1)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", o_mino.rot(2)._180(0).x, o_mino.rot(2)._180(0).y, o_mino.rot(2)._180(1).x, o_mino.rot(2)._180(1).y, o_mino.rot(2)._180(2).x, o_mino.rot(2)._180(2).y, o_mino.rot(2)._180(3).x, o_mino.rot(2)._180(3).y, o_mino.rot(2)._180(4).x, o_mino.rot(2)._180(4).y, o_mino.rot(2)._180(5).x, o_mino.rot(2)._180(5).y, o_mino.rot(2)._180(6).x, o_mino.rot(2)._180(6).y, o_mino.rot(2)._180(7).x, o_mino.rot(2)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", o_mino.rot(3)._180(0).x, o_mino.rot(3)._180(0).y, o_mino.rot(3)._180(1).x, o_mino.rot(3)._180(1).y, o_mino.rot(3)._180(2).x, o_mino.rot(3)._180(2).y, o_mino.rot(3)._180(3).x, o_mino.rot(3)._180(3).y, o_mino.rot(3)._180(4).x, o_mino.rot(3)._180(4).y, o_mino.rot(3)._180(5).x, o_mino.rot(3)._180(5).y, o_mino.rot(3)._180(6).x, o_mino.rot(3)._180(6).y, o_mino.rot(3)._180(7).x, o_mino.rot(3)._180(7).y);
	fprintf(out, "		{{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(0)._180(0).x, z_mino.rot(0)._180(0).y, z_mino.rot(0)._180(1).x, z_mino.rot(0)._180(1).y, z_mino.rot(0)._180(2).x, z_mino.rot(0)._180(2).y, z_mino.rot(0)._180(3).x, z_mino.rot(0)._180(3).y, z_mino.rot(0)._180(4).x, z_mino.rot(0)._180(4).y, z_mino.rot(0)._180(5).x, z_mino.rot(0)._180(5).y, z_mino.rot(0)._180(6).x, z_mino.rot(0)._180(6).y, z_mino.rot(0)._180(7).x, z_mino.rot(0)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(1)._180(0).x, z_mino.rot(1)._180(0).y, z_mino.rot(1)._180(1).x, z_mino.rot(1)._180(1).y, z_mino.rot(1)._180(2).x, z_mino.rot(1)._180(2).y, z_mino.rot(1)._180(3).x, z_mino.rot(1)._180(3).y, z_mino.rot(1)._180(4).x, z_mino.rot(1)._180(4).y, z_mino.rot(1)._180(5).x, z_mino.rot(1)._180(5).y, z_mino.rot(1)._180(6).x, z_mino.rot(1)._180(6).y, z_mino.rot(1)._180(7).x, z_mino.rot(1)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}},\n", z_mino.rot(2)._180(0).x, z_mino.rot(2)._180(0).y, z_mino.rot(2)._180(1).x, z_mino.rot(2)._180(1).y, z_mino.rot(2)._180(2).x, z_mino.rot(2)._180(2).y, z_mino.rot(2)._180(3).x, z_mino.rot(2)._180(3).y, z_mino.rot(2)._180(4).x, z_mino.rot(2)._180(4).y, z_mino.rot(2)._180(5).x, z_mino.rot(2)._180(5).y, z_mino.rot(2)._180(6).x, z_mino.rot(2)._180(6).y, z_mino.rot(2)._180(7).x, z_mino.rot(2)._180(7).y);
	fprintf(out, "		{{%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}, {%d, %d}}},\n", z_mino.rot(3)._180(0).x, z_mino.rot(3)._180(0).y, z_mino.rot(3)._180(1).x, z_mino.rot(3)._180(1).y, z_mino.rot(3)._180(2).x, z_mino.rot(3)._180(2).y, z_mino.rot(3)._180(3).x, z_mino.rot(3)._180(3).y, z_mino.rot(3)._180(4).x, z_mino.rot(3)._180(4).y, z_mino.rot(3)._180(5).x, z_mino.rot(3)._180(5).y, z_mino.rot(3)._180(6).x, z_mino.rot(3)._180(6).y, z_mino.rot(3)._180(7).x, z_mino.rot(3)._180(7).y);
	fprintf(out, "	};\n");
	fprintf(out, "	inline constexpr const Minos::Coord *clockwise(int m, int r) {\n");
	fprintf(out, "		return cw_offset[m][r];\n");
	fprintf(out, "	}\n");
	fprintf(out, "	inline constexpr const Minos::Coord *counterclockwise(int m, int r) {\n");
	fprintf(out, "		return ccw_offset[m][r];\n");
	fprintf(out, "	}\n");
	fprintf(out, "	inline constexpr const Minos::Coord *_180_spin(int m, int r) {\n");
	fprintf(out, "		return _180_offset[m][r];\n");
	fprintf(out, "	}\n");
	fprintf(out, "	constexpr int cw_size[7][4] = {\n");
	fprintf(out, "		{%d, %d, %d, %d},\n", s_mino.rot(0).cw_max(), s_mino.rot(1).cw_max(), s_mino.rot(2).cw_max(), s_mino.rot(3).cw_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", l_mino.rot(0).cw_max(), l_mino.rot(1).cw_max(), l_mino.rot(2).cw_max(), l_mino.rot(3).cw_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", z_mino.rot(0).cw_max(), z_mino.rot(1).cw_max(), z_mino.rot(2).cw_max(), z_mino.rot(3).cw_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", i_mino.rot(0).cw_max(), i_mino.rot(1).cw_max(), i_mino.rot(2).cw_max(), i_mino.rot(3).cw_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", t_mino.rot(0).cw_max(), t_mino.rot(1).cw_max(), t_mino.rot(2).cw_max(), t_mino.rot(3).cw_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", o_mino.rot(0).cw_max(), o_mino.rot(1).cw_max(), o_mino.rot(2).cw_max(), o_mino.rot(3).cw_max());
	fprintf(out, "		{%d, %d, %d, %d}\n", j_mino.rot(0).cw_max(), j_mino.rot(1).cw_max(), j_mino.rot(2).cw_max(), j_mino.rot(3).cw_max());
	fprintf(out, "	};\n");
	fprintf(out, "	constexpr int ccw_size[7][4] = {\n");
	fprintf(out, "		{%d, %d, %d, %d},\n", s_mino.rot(0).ccw_max(), s_mino.rot(1).ccw_max(), s_mino.rot(2).ccw_max(), s_mino.rot(3).ccw_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", l_mino.rot(0).ccw_max(), l_mino.rot(1).ccw_max(), l_mino.rot(2).ccw_max(), l_mino.rot(3).ccw_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", z_mino.rot(0).ccw_max(), z_mino.rot(1).ccw_max(), z_mino.rot(2).ccw_max(), z_mino.rot(3).ccw_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", i_mino.rot(0).ccw_max(), i_mino.rot(1).ccw_max(), i_mino.rot(2).ccw_max(), i_mino.rot(3).ccw_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", t_mino.rot(0).ccw_max(), t_mino.rot(1).ccw_max(), t_mino.rot(2).ccw_max(), t_mino.rot(3).ccw_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", o_mino.rot(0).ccw_max(), o_mino.rot(1).ccw_max(), o_mino.rot(2).ccw_max(), o_mino.rot(3).ccw_max());
	fprintf(out, "		{%d, %d, %d, %d}\n", j_mino.rot(0).ccw_max(), j_mino.rot(1).ccw_max(), j_mino.rot(2).ccw_max(), j_mino.rot(3).ccw_max());
	fprintf(out, "	};\n");
	fprintf(out, "	constexpr int _180_size[7][4] = {\n");
	fprintf(out, "		{%d, %d, %d, %d},\n", s_mino.rot(0)._180_max(), s_mino.rot(1)._180_max(), s_mino.rot(2)._180_max(), s_mino.rot(3)._180_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", l_mino.rot(0)._180_max(), l_mino.rot(1)._180_max(), l_mino.rot(2)._180_max(), l_mino.rot(3)._180_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", z_mino.rot(0)._180_max(), z_mino.rot(1)._180_max(), z_mino.rot(2)._180_max(), z_mino.rot(3)._180_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", i_mino.rot(0)._180_max(), i_mino.rot(1)._180_max(), i_mino.rot(2)._180_max(), i_mino.rot(3)._180_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", t_mino.rot(0)._180_max(), t_mino.rot(1)._180_max(), t_mino.rot(2)._180_max(), t_mino.rot(3)._180_max());
	fprintf(out, "		{%d, %d, %d, %d},\n", o_mino.rot(0)._180_max(), o_mino.rot(1)._180_max(), o_mino.rot(2)._180_max(), o_mino.rot(3)._180_max());
	fprintf(out, "		{%d, %d, %d, %d}\n", j_mino.rot(0)._180_max(), j_mino.rot(1)._180_max(), j_mino.rot(2)._180_max(), j_mino.rot(3)._180_max());
	fprintf(out, "	};\n");
	fprintf(out, "	inline constexpr const int& clockwise_size(int m, int r) {\n");
	fprintf(out, "		return cw_size[m][r];\n");
	fprintf(out, "	}\n");
	fprintf(out, "	inline constexpr const int& counter_clockwise_size(int m, int r) {\n");
	fprintf(out, "		return ccw_size[m][r];\n");
	fprintf(out, "	}\n");
	fprintf(out, "	inline constexpr const int& _180_spin_size(int m, int r) {\n");
	fprintf(out, "		return _180_size[m][r];\n");
	fprintf(out, "	}\n");
	fprintf(out, "}\n");
	fclose(out);
	printf("Complete.\n");
	return 0;
}