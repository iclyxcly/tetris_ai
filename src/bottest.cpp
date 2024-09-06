#include "tetris_core.h"
#include <random>
#include <stdio.h>
#include <queue>
#include <iostream>

using namespace TetrisAI;

void read_config(TetrisParam &param)
{
    FILE *file = fopen("best_param.txt", "r");
    if (file == nullptr)
    {
        return;
    }
    for (int i = 0; i < END_OF_PARAM; ++i)
    {
        fscanf(file, "%lf\n", &param.weight[i]);
    }
    fclose(file);
}

std::queue<uint8_t> generate_bag()
{
    std::queue<uint8_t> bag;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<uint8_t> pool = {S, L, Z, I, T, O, J};
    while (!pool.empty())
    {
        std::uniform_int_distribution<> dis(0, pool.size() - 1);
        uint8_t index = dis(gen);
        bag.push(pool[index]);
        pool.erase(pool.begin() + index);
    }
    return bag;
}

int main(void)
{
    TetrisConfig config;
    config.target_time = 100;
    TetrisNextManager next(config);
    TetrisMap map(10, 25);
    std::random_device rd;
    std::mt19937 gen(rd());
    TetrisMinoManager mino_manager("botris_srs.json");
    std::uniform_int_distribution<> line_dis(2, 3);
    std::uniform_int_distribution<> piece_dis(2, 5);
    int count = 0;
    int total_atk = 0;
    int total = 0;
    uint8_t extra = 0;
    int total_recv = 0;
    TetrisParam param;
    TetrisStatus status;
    while (true)
    {
        read_config(param);
        if (next.queue.size() < 7)
        {
            auto bag = generate_bag();
            next.insert(bag);
        }
        next.next();
        status.next = next;
        TetrisTree tree(map, status, config, param);
        auto result = tree.run();
        TetrisGameEmulation emu;
        emu.run(map, next, status, result);
        
        if (count % piece_dis(gen) == 0)
        {
            uint8_t recv = line_dis(gen);
            total_recv += recv;
            status.garbage.push_lines(recv, (uint8_t)1);
            status.garbage.fight_lines(extra);
        }
        ++count;
        total += status.clear;
        total_atk += status.attack;
        extra += status.send_attack;
        if (status.dead)
        {
            map = TetrisMap(10, 40);
            next.hold = EMPTY;
            next.queue = std::queue<uint8_t>();
            status.init();
            total_atk = 0;
            total = 0;
            count = 0;
            extra = 0;
            total_recv = 0;
        }
        for (int i = config.default_y + 4; i >= 0; i--)
        {
            printf("%2d |", i);
            for (int j = 0; j < map.width; j++)
            {
                printf("%s", (map.board[i] >> j) & 1 ? "[]" : "  ");
            }
            printf("|\n");
        }
        printf("#%d\n", count);
        printf("version: %ld, total_nodes: %ld\n", tree.stable_version, tree.total_nodes);
        printf("hold: %c, queue: ", type_to_char[next.hold]);
        auto queue = next.queue;
        while (!queue.empty())
        {
            printf("%c", type_to_char[queue.front()]);
            queue.pop();
        }
        printf("\n");
        printf("b2b: %d, combo: %d, clear: %d, spin_type: %d, app: %.2f, apl: %.2f, opponent app: %.2f\n", status.b2b, status.combo, status.clear, status.allspin, total_atk / (double)count, total_atk / (double)total, total_recv / (double)count);
        printf("cur_atk: %d, send_atk: %d\n", status.attack, status.send_attack);
        printf("path: %s\n", result.c_str());
    }
    return 0;
}