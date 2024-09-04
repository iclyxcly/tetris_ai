#include "tetris_core.h"
#include <random>
#include <stdio.h>
#include <queue>

using namespace TetrisAI;

void read_config(TetrisParam &param)
{
    FILE *file = fopen("best_param.txt", "r");
    if (file == NULL)
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
    std::map<uint8_t, char> type_to_char = {
        {S, 'S'},
        {L, 'L'},
        {Z, 'Z'},
        {I, 'I'},
        {T, 'T'},
        {O, 'O'},
        {J, 'J'},
        {EMPTY, ' '}};
    TetrisConfig config;
    config.target_time = 1;
    config.can_hold = true;
    TetrisNextManager next(config);
    TetrisMap map(10, 40);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, map.width - 1);
    std::uniform_int_distribution<> mess_dis(0, 5);
    TetrisMinoManager mino_manager("botris_srs.json");
    TetrisPendingLineManager pending(dis, mess_dis, gen);
    std::uniform_int_distribution<> line_dis(2, 3);
    std::uniform_int_distribution<> piece_dis(2, 2);
    int16_t b2b = 0, combo = 0;
    uint8_t clear = 0, spin_type = 0;
    int count = 0;
    int total_atk = 0;
    int total = 0;
    int extra = 0;
    int total_recv = 0;
    TetrisParam param;
    while (true)
    {
        int attack = 0;
        read_config(param);
        if (next.queue.size() < 7)
        {
            auto bag = generate_bag();
            next.insert(bag);
        }
        next.next();
        map.scan();
        TetrisStatus status(b2b, combo, next, pending);
        TetrisTree tree(map, status, param);
        auto result = tree.run();
        pending.decay();
        result += "V";
        if (result[0] == 'v')
        {
            next.change_hold();
        }
        if (count % piece_dis(gen) == 0)
        {
            int recv = line_dis(gen);
            total_recv += recv;
            pending.push_lines(recv, 1);
            pending.fight_lines(extra);
        }
        TetrisInstructor instructor(map, next.active.type);
        TetrisActive next_active(config.default_x, config.default_y, config.default_r, next.queue.front());
        for (auto &path : result)
        {
            switch (path)
            {
            case 'v':
                break;
            case 'V':
                instructor.build_snapshot(next.active);
                spin_type = instructor.immobile(next.active) ? 3 : 0;
                instructor.attach(map, next.active);
                clear = map.flush();
                map.scan();
                break;
            case 'l':
                instructor.l(next.active);
                break;
            case 'r':
                instructor.r(next.active);
                break;
            case 'L':
                instructor.L(next.active);
                break;
            case 'R':
                instructor.R(next.active);
                break;
            case 'd':
                instructor.d(next.active);
                break;
            case 'D':
                instructor.D(next.active);
                break;
            case 'x':
                instructor.x(next.active);
                break;
            case 'c':
                instructor.c(next.active);
                break;
            case 'z':
                instructor.z(next.active);
                break;
            }
        }
        switch (clear)
        {
        case 0:
            combo = 0;
            pending.take_all_damage(map, atk.messiness);
            break;
        case 1:
            if (spin_type == 3)
            {
                attack += atk.ass + b2b;
                b2b = 1;
            }
            else
            {
                b2b = 0;
            }
            attack += atk.combo_table[++combo];
            break;
        case 2:
            if (spin_type == 3)
            {
                attack += atk.asd + b2b;
                b2b = 1;
            }
            else
            {
                b2b = 0;
                attack += 1;
            }
            attack += atk.combo_table[++combo];
            break;
        case 3:
            if (spin_type == 3)
            {
                attack += atk.ast + b2b;
                b2b = 1;
            }
            else
            {
                b2b = 0;
                attack += 2;
            }
            attack += atk.combo_table[++combo];
            break;
        case 4:
            attack += 4 + b2b;
            b2b = 1;
            attack += atk.combo_table[++combo];
            break;
        }
        ++count;
        total += clear;
        total_atk += attack;
        extra += attack;
        pending.fight_lines(extra);
        if (instructor.check_death(map, next_active))
        {
            map = TetrisMap(10, 40);
            next.hold = EMPTY;
            next.queue = std::queue<uint8_t>();
            clear = 0;
            b2b = 0;
            combo = 0;
            spin_type = 0;
            total_atk = 0;
            total = 0;
            count = 0;
            pending.pending = std::deque<TetrisPendingLine>();
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
        printf("b2b: %d, combo: %d, clear: %d, spin_type: %d, app: %.2f, apl: %.2f, opponent app: %.2f\n", b2b, combo, clear, spin_type, total_atk / (double)count, total_atk / (double)total, total_recv / (double)count);
        printf("path: %s\n", result.c_str());
        usleep(100000);
    }
    return 0;
}