#include <random>
#include "json.hpp"
#include <bitset>
#include <fstream>
#include <map>
#include <vector>
#include <iostream>
#include "tetris_core.h"
#include "stdio.h"

using namespace TetrisAI;

int main()
{
    TetrisAI::TetrisMinoManager mino_manager("botris_srs.json");
    auto mino_list = mino_manager.get();

    std::map<int, std::string> type_to_string_cw = {
        {0, "L->0"},
        {1, "0->R"},
        {2, "R->2"},
        {3, "2->L"}};

    std::map<int, std::string> type_to_string_ccw = {
        {0, "R->0"},
        {1, "0->L"},
        {2, "L->2"},
        {3, "2->R"}};

    for (auto &mino : mino_list)
    {
        printf("type: %d\n", mino.first);

        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                printf("|");
                for (int k = 0; k < 4; k++)
                {
                    printf("%s", ((mino.second.data[i][j] >> k) & 1) ? "[]" : "  ");
                }
                printf("|\n");
            }
            printf("\n");
        }

        for (int i = 0; i < 4; i++)
        {
            printf("%s\n", type_to_string_cw[i].c_str());
            printf("up_offset: %d\n", mino.second.up_offset[i]);
            printf("right_offset: %d\n", mino.second.right_offset[i]);
            printf("down_offset: %d\n", mino.second.down_offset[i]);
            printf("left_offset: %d\n", mino.second.left_offset[i]);
        }

        printf("\n");

        for (int i = 0; i < 4; i++)
        {
            printf("cw %s\n", type_to_string_cw[i].c_str());
            for (int j = 0; j < mino.second.rotate_right[i].size(); j++)
            {
                for (int k = 0; k < mino.second.rotate_right[i][j].size(); k++)
                {
                    printf("(%d, %d) ", mino.second.rotate_right[i][j][k].first, mino.second.rotate_right[i][j][k].second);
                }
                printf("\n");
            }
            printf("\n");
        }

        printf("\n");

        for (int i = 0; i < 4; i++)
        {
            printf("ccw %s\n", type_to_string_ccw[i].c_str());
            for (int j = 0; j < mino.second.rotate_left[i].size(); j++)
            {
                for (int k = 0; k < mino.second.rotate_left[i][j].size(); k++)
                {
                    printf("(%d, %d) ", mino.second.rotate_left[i][j][k].first, mino.second.rotate_left[i][j][k].second);
                }
                printf("\n");
            }
            printf("\n");
        }
    }

        auto &xd = mino_manager.get_move_cache()[TetrisAI::TetrisMinoType::I];
        for (int i = 0; i < 4; i++)
        {
            printf("cw %s\n", type_to_string_cw[i].c_str());
            for (auto &j : xd[i])
            {
                for (int k = 0; k < 4; k++)
                {
                    printf("%s\n", std::bitset<32>(j.second[k]).to_string().c_str());
                }
            printf("\n");
            }
            printf("\n");
        }

    return 0;
}
