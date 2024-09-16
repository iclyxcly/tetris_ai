#include "engine.hpp"
#include <random>
#include <stdio.h>
#include <queue>
#include <iostream>
#pragma warning(disable : 4996)
using namespace moenew;

int main(void)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> line_dis(2, 3);
    std::uniform_int_distribution<> piece_dis(2, 5);
    int count = 0;
    int total_atk = 0;
    int total = 0;
    uint8_t extra = 0;
    int total_recv = 0;
    Engine engine;
    auto status = engine.get_board_status();
    auto &p = engine.get_param();
    auto &atk = engine.get_attack_table();
    atk.aspin_1 = 2;
    atk.aspin_2 = 4;
    atk.aspin_3 = 6;
    atk.clear_1 = 0;
    atk.clear_2 = 1;
    atk.clear_3 = 2;
    atk.clear_4 = 4;
    atk.pc = 10;
    atk.b2b = 1;
    memset(atk.combo, 0, sizeof(atk.combo));
    while (true)
    {
        status.next.fill();
        auto mino_loc = engine.get_mino_draft();
        engine.submit_form(mino_loc, status, true);
        auto result = engine.start();
        if (result.first)
        {
            status.next.swap();
        }
        printf("x: %d, y: %d, r: %d\n", result.second.get_x(), result.second.get_y(), result.second.get_r());
        status.board.paste(cache_get(status.next.pop(), result.second.get_r(), result.second.get_x()), result.second.get_y());
        std::cout << status.board.print(22);
        Sleep(1000);
    }
    return 0;
}