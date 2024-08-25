#include "tetris_core.h"
#include <random>
#include <stdio.h>
#include <queue>

using namespace TetrisAI;

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
    while (true)
    {
        if (next.queue.size() < 7)
        {
            auto bag = generate_bag();
            next.insert(bag);
        }
        next.next();
        TetrisActive active(next.active);
        TetrisStatus status(b2b, combo, next, pending);
        TetrisTree tree(map, config, status);
        auto result = tree.run();
        result.front().path += "V";
        TetrisInstructor instructor(map, active.type);
        TetrisActive next_active(config.default_x, config.default_y, config.default_r, next.queue.front());
        for (auto &path : result.front().path)
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
                    map = TetrisMap(10, 40);
                    next.hold = EMPTY;
                    next.queue = std::queue<uint8_t>();
                    clear = 0;
                    b2b = 0;
                    combo = 0;
                    spin_type = 0;
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
        for (int i = config.default_y + 4; i >= 0; i--)
        {
            printf("%2d |", i);
            for (int j = 0; j < map.width; j++)
            {
                printf("%s", (map.board[i] >> j) & 1 ? "[]" : "  ");
            }
            printf("|\n");
        }
        printf("hold: %c, queue: ", type_to_char[next.hold]);
        auto queue = next.queue;
        while (!queue.empty())
        {
            printf("%c", type_to_char[queue.front()]);
            queue.pop();
        }
        printf("\n");
        printf("b2b: %d, combo: %d, clear: %d, spin_type: %d\n", b2b, combo, clear, spin_type);
        printf("path: %s\n", result.front().path.c_str());
    }
    return 0;
}