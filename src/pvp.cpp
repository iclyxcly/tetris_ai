#include "engine.hpp"
#include "movegen.hpp"
#include "emu.hpp"
#include <random>
#include <stdio.h>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <atomic>

using namespace moenew;
using Playstyle = Evaluation::Playstyle;

struct TetrisPlayer
{
    Playstyle param;
    Engine engine;
    Next real_next;
    Evaluation::Status status;
    Random rand;
    int next;
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
        status.under_attack.push(lines, 1);
        receive += lines;
    }

    TetrisPlayer(Playstyle &param, uint64_t seed)
        : count(0), clear(0), attack(0), receive(0), rand(seed)
    {
        status.reset();
        engine.get_param() = param;
        this->param = param;
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
        next = 7;
    }

    bool run()
    {
        if (count > 220)
            if (count % 11 == 0)
            {
                engine.get_attack_table().multiplier += 0.1;
            }
        real_next.fill();
        status.next.next = real_next.get(next);
        status.next.hold = real_next.hold;
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
        ++count;
        attack += status.attack;
        clear += status.clear;
        return !status.dead;
    }
};

void read_config(Playstyle &param, std::string path)
{
    FILE *file = fopen(path.c_str(), "r");
    if (file == nullptr)
    {
        utils::println(utils::ERR, " -> Failed to open " + path);
        exit(1);
    }
    for (int i = 0; i < Evaluation::END_OF_PARAM; ++i)
    {
        fscanf(file, "%lf\n", &param[i]);
    }
    fclose(file);
}

void view(TetrisPlayer &p1, TetrisPlayer &p2, int win[2])
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
    snprintf(out, sizeof out, "[P1]: #%4d HOLD = %c NEXT = %s UP = %d COMBO = %d B2B = %d, APP = %3.2f, WIN = %d, COUNT = %d\n"
                              "[P2]: #%4d HOLD = %c NEXT = %s UP = %d COMBO = %d B2B = %d, APP = %3.2f, WIN = %d, COUNT = %d\n",
             p1.count, type_to_char(p1.real_next.hold), nexts_1.c_str(), up_1, p1.status.combo, p1.status.b2b, (double)p1.attack / (double)p1.count, win[0], p1.status.board.count,
             p2.count, type_to_char(p2.real_next.hold), nexts_2.c_str(), up_2, p2.status.combo, p2.status.b2b, (double)p2.attack / (double)p2.count, win[1], p2.status.board.count);
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

bool run_parallel(TetrisPlayer &p1, TetrisPlayer &p2)
{
    static std::recursive_mutex mutex;
    std::vector<std::thread> threads;
    for (int i = 0; i < 2; ++i)
    {
        threads.push_back(std::thread([&, i](TetrisPlayer &player)
                                        { 
                                            player.run();
                                            if (player.status.send_attack)
                                            {
                                                mutex.lock();
                                                if (i)
                                                {
                                                    p1.push_damage(player.status.send_attack);
                                                }
                                                else
                                                {
                                                    p2.push_damage(player.status.send_attack);
                                                }
                                                player.status.send_attack = 0;
                                                mutex.unlock();
                                            }
                                        }, i ? std::ref(p2) : std::ref(p1)));
    }
    for (auto &i : threads)
    {
        i.join();
    }
    return !p1.status.dead && !p2.status.dead;
}

void read_config(Playstyle &p1, Playstyle &p2)
{
    read_config(p1, "best_param.txt");
    read_config(p2, "best_param_2.txt");
}

int main()
{
    Playstyle p1, p2;
    read_config(p1, p2);
    int wins[2] = {0, 0};
    while (true)
    {
        TetrisPlayer player_1(p1, rand());
        TetrisPlayer player_2(p2, rand());
        while (run_parallel(player_1, player_2))
        {
            Playstyle _p1, _p2;
            read_config(_p1, _p2);
            if (_p1 != p1 || _p2 != p2)
            {
                p1 = _p1;
                p2 = _p2;
                wins[0] = 0;
                wins[1] = 0;
                break;
            }
            view(player_1, player_2, wins);
        }
        if (player_1.status.dead)
        {
            ++wins[1];
        }
        else if (player_2.status.dead)
        {
            ++wins[0];
        }
        if (wins[0] == 15 || wins[1] == 15)
        {
            std::cin.get();
            wins[0] = 0;
            wins[1] = 0;
        }
    }
}