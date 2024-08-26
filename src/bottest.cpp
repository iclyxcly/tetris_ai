#include "tetris_core.h"
#include <random>
#include <stdio.h>
#include <queue>

using namespace TetrisAI;

void read_config() {
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
    config.target_time = 100;
    config.can_hold = true;
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
    int count = 0;
    int attack = 0;
    int total = 0;
    while (true)
    {
        read_config();
        if (next.queue.size() < 7)
        {
            auto bag = generate_bag();
            next.insert(bag);
        }
        next.next();
        map.scan();
        TetrisStatus status(b2b, combo, next, pending);
        TetrisTree tree(map, config, status);
        auto result = tree.run();
        result.front().path += "V";
        if (result.front().path[0] == 'v')
        {
            next.change_hold();
        }
        TetrisInstructor instructor(map, next.active.type);
        TetrisActive next_active(config.default_x, config.default_y, config.default_r, next.queue.front());
        for (auto &path : result.front().path)
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
            attack += atk.combo_table[combo++];
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
            attack += atk.combo_table[combo++];
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
            attack += atk.combo_table[combo++];
            break;
        case 4:
            attack += 4 + b2b;
            b2b = 1;
            attack += atk.combo_table[combo++];
            break;
        }
        ++count;
        total += clear;
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
        printf("b2b: %d, combo: %d, clear: %d, spin_type: %d, app: %.2f, apl: %.2f\n", b2b, combo, clear, spin_type, attack / (double)count, attack / (double)total);
        printf("path: %s\n", result.front().path.c_str());
    }
    return 0;
}