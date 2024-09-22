#include "engine.hpp"
#include "board.hpp"
#include "next.hpp"
#include "emu.hpp"
#include "pending.hpp"
#include "eval.hpp"
#include "utils.hpp"
#include <stdio.h>
#include <thread>
#include <iostream>

using namespace moenew;
using Playstyle = Evaluation::Playstyle;
constexpr int CYCLE_MAX = 1000;
struct Chemistry
{
    double precision = 1;
    int cycle = 0;
    int cycle_index = 0;
    Playstyle base;
    void load_init()
    {
        memset(base.param, 0, sizeof(base));
        FILE *file = fopen("import_param.txt", "r");
        if (file == nullptr)
        {
            return;
        }
        for (int i = 0; i < Evaluation::END_OF_PARAM; ++i)
        {
            fscanf(file, "%lf", &base[i]);
        }
        fclose(file);
    }
    bool try_load_data()
    {
        FILE *file = fopen("chemistry.bin", "rb");
        if (file == nullptr)
        {
            return false;
        }
        fread(&precision, sizeof(precision), 1, file);
        fread(&cycle, sizeof(cycle), 1, file);
        fread(&cycle_index, sizeof(cycle_index), 1, file);
        fread(&base, sizeof(base), 1, file);
        fclose(file);
        return true;
    }
    void export_best()
    {
        FILE *file = fopen("best_param.txt", "w");
        for (int i = 0; i < Evaluation::END_OF_PARAM; ++i)
        {
            fprintf(file, "%lf\n", base[i]);
        }
        fclose(file);
    }
    void export_data()
    {
        FILE *file = fopen("chemistry.bin", "wb");
        fwrite(&precision, sizeof(precision), 1, file);
        fwrite(&cycle, sizeof(cycle), 1, file);
        fwrite(&cycle_index, sizeof(cycle_index), 1, file);
        fwrite(&base, sizeof(base), 1, file);
        fclose(file);
    }
};
struct TetrisPlayer
{
    Engine engine;
    FakeNext fake_next;
    Next real_next;
    Evaluation::Status status;
    Random rand;
    int next_length;
    int count;
    int clear;
    int attack;
    int receive;

    void push_damage(uint8_t lines)
    {
        if (lines == 0)
        {
            return;
        }
        bool push_1 = !status.under_attack.empty() || !(rand.next() % 3);
        status.under_attack.push(lines, push_1);
        receive += lines;
    }

    TetrisPlayer(Playstyle &param, uint64_t &seed)
        : count(0), clear(0), attack(0), receive(0), rand(seed)
    {
        status.reset();
        engine.get_param() = param;
        auto &atk = engine.get_attack_table();
        atk.messiness = 0.05;
        atk.aspin_1 = 2;
        atk.aspin_2 = 4;
        atk.aspin_3 = 6;
        atk.clear_1 = 0;
        atk.clear_2 = 1;
        atk.clear_3 = 2;
        atk.clear_4 = 4;
        atk.pc = 10;
        atk.b2b = 1;
        atk.multiplier = 1;
        int combo_table[21] = {0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
        memcpy(atk.combo, combo_table, sizeof(atk.combo));
        next_length = 5;
    }

    bool run()
    {
        if (count > 200)
            if (count % 11 == 0)
            {
                engine.get_attack_table().multiplier += 0.1;
            }
        real_next.fill();
        status.next.next = real_next.get(next_length);
        status.next.hold = real_next.hold;
        status.next.fill(fake_next);
        auto mino_loc = engine.get_mino_draft();
        engine.submit_form(mino_loc, status, true);
        auto result = engine.start_depth_thread();
        status.under_attack.rngify();
        if (result.change_hold)
        {
            real_next.swap();
            status.next.swap();
        }
        if (!cycle(status, result, engine.get_attack_table()))
        {
            status.dead = true;
        }
        real_next.pop();
        fake_next.pop();
        ++count;
        attack += status.attack;
        clear += status.clear;
        return !status.dead;
    }
};

void view(TetrisPlayer &p1, TetrisPlayer &p2, std::atomic<int> win[2])
{
    char out[81920] = "";
    char box_0[3] = "  ";
    char box_1[3] = "[]";

    out[0] = '\0';
    std::string nexts_1 = p1.real_next.to_string();
    std::string nexts_2 = p2.real_next.to_string();
    nexts_1.resize(6);
    nexts_2.resize(6);
    uint16_t up_1 = p1.status.under_attack.total();
    uint16_t up_2 = p2.status.under_attack.total();
    snprintf(out, sizeof out, "[TEST]: HOLD = %c NEXT = %s UP = %d COMBO = %d B2B = %d, APP = %3.2f, WIN = %d\n"
                              "[BASE]: HOLD = %c NEXT = %s UP = %d COMBO = %d B2B = %d, APP = %3.2f, WIN = %d\n",
             type_to_char((Piece)p1.real_next.hold), nexts_1.c_str(), up_1, p1.status.combo, p1.status.b2b, (double)p1.attack / (double)p1.count, win[0].load(),
             type_to_char((Piece)p2.real_next.hold), nexts_2.c_str(), up_2, p2.status.combo, p2.status.b2b, (double)p2.attack / (double)p2.count, win[1].load());
    Board map_copy1 = p1.status.board;
    Board map_copy2 = p2.status.board;
    map_copy1.paste(cache_get(p1.status.next.peek(), DEFAULT_R, DEFAULT_X), DEFAULT_Y);
    map_copy2.paste(cache_get(p2.status.next.peek(), DEFAULT_R, DEFAULT_X), DEFAULT_Y);
    for (int y = DEFAULT_Y + 4; y >= 0; --y)
    {
        snprintf(out + strlen(out), sizeof out - strlen(out), "%2d|", y);
        for (int x = 0; x < 10; ++x)
        {
            strcat(out, map_copy1.get(x, y) ? box_1 : box_0);
        }
        snprintf(out + strlen(out), sizeof out - strlen(out), "|  %2d|", y);
        for (int x = 0; x < 10; ++x)
        {
            strcat(out, map_copy2.get(x, y) ? box_1 : box_0);
        }
        strcat(out, "|\r\n");
    }
    printf("%s", out);
};

bool test_case(Playstyle &p1, Playstyle &p2)
{
    constexpr int FT = 15;
    std::atomic<int> win[2] = {0, 0};
    int count = 0;
    int view_index = 0;
    while (win[0] < FT && win[1] < FT)
    {
        std::vector<std::thread> threads;
        int min = std::min(FT - win[0], FT - win[1]);
        for (int i = 0; i < min; ++i)
            threads.emplace_back([&p1, &p2, &win, &count, i, &view_index]
                                 {
                uint64_t seed = rand();
                TetrisPlayer player[2] = {{p1,seed}, {p2, seed}};
                while (player[0].run() && player[1].run())
                {
                    auto &p1_send = player[0].status.send_attack;
                    auto &p2_send = player[1].status.send_attack;
                    if (p1_send > p2_send)
                    {
                        player[1].push_damage(p1_send - p2_send);
                    }
                    else if (p2_send > p1_send)
                    {
                        player[0].push_damage(p2_send - p1_send);
                    }
                    if (i == view_index)
                        view(player[0], player[1], win);
                }
                if (player[0].status.dead)
                {
                    ++win[1];
                }
                else
                {
                    ++win[0];
                } });
        for (int i = 0; i < min; ++i)
        {
            view_index = i;
            threads[i].join();
        }
    }
    return win[0] > win[1];
}

bool loop_test(Playstyle &test, Playstyle &base, double &momentum, int index)
{
    test[index] += momentum;
    if (test_case(test, base))
    {
        base = test;
        momentum *= 1.1;
        test[index] += momentum;
        while (test_case(test, base))
        {
            base = test;
            test[index] += momentum;
        }
        return true;
    }
    return false;
}

int main(void)
{
    srand(time(nullptr));
    Chemistry lab;
    if (!lab.try_load_data())
    {
        lab.load_init();
    }
    bool has_best = false;
    while (true)
    {
        if (lab.cycle >= CYCLE_MAX)
        {
            printf("Complete\n");
            break;
        }
        for (auto &i = lab.cycle_index; i < Evaluation::END_OF_PARAM; ++i)
        {
            lab.export_data();
            if (has_best)
            {
                lab.export_best();
                has_best = false;
            }
            Playstyle test = lab.base;
            double momentum = lab.precision;
            if (loop_test(test, lab.base, momentum, i))
            {
                has_best = true;
            }
            else
            {
                momentum *= -1;
                if (loop_test(test, lab.base, momentum, i))
                {
                    has_best = true;
                }
            }
        }
        ++lab.cycle;
        lab.cycle_index = 0;
        lab.precision *= 0.95;
    }
}