#include "tetris_core.h"
#include <random>
#include <stdio.h>
#include <queue>
#include <unistd.h>

using namespace TetrisAI;

void read_config()
{
    std::ifstream file("param.json");
    json data;
    file >> data;
    p.roof = data["roof"];
    p.col_trans = data["col_trans"];
    p.row_trans = data["row_trans"];
    p.hole_count = data["hole_count"];
    p.hole_line = data["hole_line"];
    p.aggregate_height = data["aggregate_height"];
    p.bumpiness = data["bumpiness"];
    p.wide_2 = data["wide_2"];
    p.wide_3 = data["wide_3"];
    p.wide_4 = data["wide_4"];
    p.attack = data["attack"];
    p.b2b = data["b2b"];
    p.combo = data["combo"];
    p.clear_1 = data["clear_1"];
    p.clear_2 = data["clear_2"];
    p.clear_3 = data["clear_3"];
    p.clear_4 = data["clear_4"];
    p.aspin_1 = data["aspin_1"];
    p.aspin_2 = data["aspin_2"];
    p.aspin_3 = data["aspin_3"];
    p.aspin_slot = data["aspin_slot"];
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
    config.default_y = 10;
    TetrisNextManager next(config);
    TetrisMap map(10, 40);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::mt19937 mess_gen(rd());
    std::uniform_int_distribution<> dis(0, map.width - 1);
    std::uniform_int_distribution<> mess_dis(0, 99);
    TetrisMinoManager mino_manager("botris_srs.json");
    TetrisPendingLineManager pending(dis, mess_dis, gen, mess_gen);
    int16_t b2b = 0, combo = 0;
    uint8_t clear = 0, spin_type = 0;
    int count = 0, last = 0, highest = 0;
    int total_count = 0;
    double average = 0;
    int death_count = 0;
    read_config();
    while (true)
    {
        if (next.queue.size() < 7)
        {
            auto bag = generate_bag();
            next.insert(bag);
        }
        TetrisActive active(config.default_x, config.default_y, config.default_r, next.queue.front());
        next.next();
        TetrisStatus status(b2b, combo, next, pending);
        TetrisPathManager path(active, config, map);
        auto result = path.run_lite(map, status, active.type, config);
        result.path += "V";
        TetrisInstructor instructor(map, active.type);
        TetrisActive next_active(config.default_x, config.default_y, config.default_r, next.queue.front());
        for (auto &path : result.path)
        {
            switch (path)
            {
            case 'v':
                next.change_hold();
                break;
            case 'V':
                instructor.build_snapshot(active);
                spin_type = instructor.immobile(active) ? 3 : 0;
                instructor.attach(map, active);
                clear = map.flush();
                map.scan();
                if (instructor.check_death(next_active))
                {
                    ++death_count;
                    map = TetrisMap(10, 40);
                    next.hold = EMPTY;
                    next.queue = std::queue<uint8_t>();
                    clear = 0;
                    b2b = 0;
                    combo = 0;
                    spin_type = 0;
                    last = count;
                    if (count > highest)
                    {
                        highest = count;
                    }
                    average = total_count / death_count;
                    count = 0;
                    if (death_count == 100)
                    {
                        printf("highest: %d, average: %.2f\n", highest, average);
                        return 0;
                    }
                }
                break;
            case 'l':
                instructor.l(active);
                break;
            case 'r':
                instructor.r(active);
                break;
            case 'L':
                instructor.L(active);
                break;
            case 'R':
                instructor.R(active);
                break;
            case 'd':
                instructor.d(active);
                break;
            case 'D':
                instructor.D(active);
                break;
            case 'x':
                instructor.x(active);
                break;
            case 'c':
                instructor.c(active);
                break;
            case 'z':
                instructor.z(active);
                break;
            }
        }
        switch (clear)
        {
        case 0:
            combo = 0;
            break;
        case 1:
            ++combo;
            if (spin_type == 3)
            {
                b2b = 1;
            }
            else
            {
                b2b = 0;
            }
            break;
        case 2:
            ++combo;
            if (spin_type == 3)
            {
                b2b = 1;
            }
            else
            {
                b2b = 0;
            }
            break;
        case 3:
            ++combo;
            if (spin_type == 3)
            {
                b2b = 1;
            }
            else
            {
                b2b = 0;
            }
            break;
        case 4:
            b2b = 1;
            ++combo;
            break;
        }
        // for (int i = config.default_y + 4; i >= 0; i--)
        // {
        //     printf("%2d |", i);
        //     for (int j = 0; j < map.width; j++)
        //     {
        //         printf("%s", (map.board[i] >> j) & 1 ? "[]" : "  ");
        //     }
        //     printf("|\n");
        // }
        // printf("hold: %c, queue: ", type_to_char[next.hold]);
        // auto queue = next.queue;
        // while (!queue.empty())
        // {
        //     printf("%c", type_to_char[queue.front()]);
        //     queue.pop();
        // }
        // printf("\n%c%c\n", type_to_char[result.type], type_to_char[next_active.type]);
        // printf("\n");
        // printf("b2b: %d, combo: %d, clear: %d, spin_type: %d\n", b2b, combo, clear, spin_type);
        // printf("path: %s\n", result.path.c_str());
        printf("#%6d, last: %6d, highest: %6d, average: %.2f, death: %d\n", ++count, last, highest, average, death_count);
        ++total_count;
        // usleep(10000);
    }
    return 0;
}