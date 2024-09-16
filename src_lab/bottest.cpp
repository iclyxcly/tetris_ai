#include "engine.hpp"
#include "movegen.hpp"
#include <random>
#include <stdio.h>
#include <queue>
#include <iostream>
#pragma warning(disable : 4996)
using namespace moenew;

int main(void)
{
    srand(time(nullptr));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> line_dis(2, 3);
    std::uniform_int_distribution<> piece_dis(2, 5);
    int count = 0;
    int total_atk = 0;
    int total_clear = 0;
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
    auto now = std::chrono::high_resolution_clock::now();
    Next global_next;
    while (++count != 10000)
    {
        global_next.fill();
        status.next.next = global_next.get(5);
        status.next.hold = global_next.hold;
        status.next.fill();
        auto mino_loc = engine.get_mino_draft();
        engine.submit_form(mino_loc, status, true);
        auto result = engine.start();
        if (result.change_hold)
        {
            global_next.swap();
        }
        status.allspin = MoveGen::immobile_global(result, global_next.peek(), status.board);
        status.board.paste(cache_get(global_next.pop(), result.get_r(), result.get_x()), result.get_y());
        int clear = 0;
        int attack = 0;
        total_clear += clear = status.board.flush();
        switch (clear)
        {
        case 0:
            status.combo = 0;
            break;
        case 1:
            if (status.allspin)
            {
                attack += atk.aspin_1 + status.b2b;
                status.b2b = true;
            }
            else
            {
                attack += atk.clear_1;
                status.b2b = false;
            }
            attack += atk.get_combo(++status.combo);
            break;
        case 2:
            if (status.allspin)
            {
                attack += atk.aspin_2 + status.b2b;
                status.b2b = true;
            }
            else
            {
                attack += atk.clear_2;
                status.b2b = false;
            }
            attack += atk.get_combo(++status.combo);
            break;
        case 3:
            if (status.allspin)
            {
                attack += atk.aspin_3 + status.b2b;
                status.b2b = true;
            }
            else
            {
                attack += atk.clear_3;
                status.b2b = false;
            }
            attack += atk.get_combo(++status.combo);
            break;
        case 4:
            attack += atk.clear_4 + status.b2b;
            status.b2b = true;
            attack += atk.get_combo(++status.combo);
            break;
        }
        if (status.board.y_max == 0)
        {
            attack = atk.pc;
        }
        total_atk += attack;
        printf("APP: %.2f\n", (double)total_atk / count);
        std::cout << status.board.print(22);
        Sleep(100);
        const auto *cache_next = cache_get(global_next.peek(), DEFAULT_R, DEFAULT_X);
        if (!status.board.integrate(cache_next, DEFAULT_Y))
        {
            printf("dead\n");
            break;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    printf("speed: %f pps\n", static_cast<double>(count) / (static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - now).count()) / 1000));
    return 0;
}