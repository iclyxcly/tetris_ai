#include "engine.hpp"
#include "movegen.hpp"
#include "emu.hpp"
#include <stdio.h>
#include <queue>
#include <iostream>
#pragma warning(disable : 4996)
using namespace moenew;

void read_config(Evaluation::Playstyle &param)
{
    FILE *file = fopen("best_param.txt", "r");
    if (file == nullptr)
    {
        utils::println(utils::ERR, " -> Failed to open best_param.txt");
        return;
    }
    for (int i = 0; i < Evaluation::END_OF_PARAM; ++i)
    {
        fscanf(file, "%lf\n", &param[i]);
    }
    fclose(file);
}

int main(void)
{
    srand(time(nullptr));
    int count = 0;
    int total_atk = 0;
    int total_clear = 0;
    int total_recv = 0;
    int max_spike = 0;
    Engine engine;
    auto status = engine.get_board_status();
    status.reset();
    auto &p = engine.get_param();
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
    read_config(p);
    memcpy(atk.combo, combo_table, sizeof(atk.combo));
    auto now = std::chrono::high_resolution_clock::now();
    Next global_next;
    struct ExtraAttack
    {
        int attack;
        int lifespan;
    };
    struct
    {
        std::deque<ExtraAttack> send;
        void decay()
        {
            for (auto &i : send)
            {
                i.lifespan--;
            }
            while (!send.empty() && send.front().lifespan < 0)
            {
                send.pop_front();
            }
        }
        void opponent_fight(int &attack)
        {
            while (!send.empty() && attack)
            {
                if (send.front().attack >= attack)
                {
                    send.front().attack -= attack;
                    attack = 0;
                }
                else
                {
                    attack -= send.front().attack;
                    send.pop_front();
                }
            }
        }
        void insert(int attack, int lifespan)
        {
            if (attack == 0)
            {
                return;
            }
            send.push_back({attack, lifespan});
        }
    } extra_attack;
    while (++count != 100)
    {
        if (count % ((rand() % 2) + 1) == 0)
        {
            int line = (rand() % 3) + 1;
            total_recv += line;
            extra_attack.opponent_fight(line);
            if (line)
            {
                status.under_attack.push(line, 1);
            }
        }
        global_next.fill();
        status.next.next = global_next.get(5);
        status.next.hold = global_next.hold;
        status.next.fill();
        auto mino_loc = engine.get_mino_draft();
        engine.submit_form(mino_loc, status, true);
        auto result = engine.start_pso();
        status.under_attack.rngify();
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
            status.under_attack.accept(status.board, atk.messiness);
            status.under_attack.decay();
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
        status.clear = clear;
        status.attack = attack;
        status.send_attack = attack;
        status.under_attack.cancel(status.send_attack);
        extra_attack.decay();
        extra_attack.insert(attack, 1);
        total_atk += attack;
        printf("APP: %.2f, Count: %d, Opponent APP: %.2f, Max Spike: %d\n", static_cast<double>(total_atk) / static_cast<double>(count), count, static_cast<double>(total_recv) / static_cast<double>(count), max_spike);
        std::cout << status.board.print(22);
        if (attack)
        {
            status.cumulative_attack += attack;
            status.attack_since = 0;
        }
        else
        {
            max_spike = std::max(max_spike, status.cumulative_attack);
            status.cumulative_attack = 0;
            ++status.attack_since;
        }
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