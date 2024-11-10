#include "engine.hpp"
#include "movegen.hpp"
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
    int id;

    void push_damage(uint8_t lines)
    {
        if (lines == 0)
        {
            return;
        }
        bool push_1 = !status.under_attack.empty() || !(rand.next() % 3) || status.clear != 0;
        status.under_attack.push(lines, push_1);
        receive += lines;
    }

    TetrisPlayer(Playstyle &param, uint64_t seed, int id)
        : count(0), clear(0), attack(0), receive(0), rand(seed)
    {
        this->id = id;
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
        if (++count % 50 == 0)
        {
            ++engine.get_attack_table().multiplier;
        }
        if (count % 5 == 0)
        {
            push_damage(1 * engine.get_attack_table().multiplier);
        }
        real_next.fill();
        status.next.next = real_next.get(next);
        status.next.hold = real_next.hold;
        auto mino_loc = engine.get_mino_draft();
        engine.submit_form(mino_loc, status, true, id);
        auto raw_result = engine.start_threaded(100);
        status = raw_result.status;
        status.under_attack.rngify();
        auto result = raw_result.decision;
        if (result.change_hold)
        {
            real_next.swap();
        }
        real_next.pop();
        attack += status.send_attack / engine.get_attack_table().multiplier;
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

void view(TetrisPlayer &p1, int rounds)
{
    std::string out = p1.status.board.print(22);
    printf("Round: %d\n", rounds);
    printf("SPP: %3.2f\n", (double)p1.attack / (double)p1.count);
    std::cout << out << std::endl;
};

bool run(TetrisPlayer &p1)
{
    p1.run();
    return !p1.status.dead;
}

int main()
{
    Playstyle p1;
    int rounds = 0;
    std::vector<double> app_1, count_1;

    while (true)
    {
        TetrisPlayer player_1(p1, rand(), 0);
        while (run(player_1))
            view(player_1, rounds);
        ++rounds;
        app_1.push_back((double)player_1.attack / (double)player_1.count);
        count_1.push_back((double)player_1.count);
        if (rounds == 10)
        {
            double avg1 = 0;
            double avg2 = 0;
            for (auto &i : app_1)
            {
                avg1 += i;
            }
            for (auto &i : count_1)
            {
                avg2 += i;
            }
            avg1 /= app_1.size();
            avg2 /= count_1.size();
            printf("SPP: %3.2f\n", avg1);
            printf("Count: %3.2f\n", avg2);
            printf("SPP/Count: %3.2f\n", avg1 / avg2);
            return 0;
        }
    }
}