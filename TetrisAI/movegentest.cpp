#include "tetris_core.h"
#include <chrono>
int main()
{
    using namespace TetrisAI;
    TetrisConfig config;
    TetrisActive active(config.default_x, config.default_y, config.default_r, L);
    TetrisMap map(10, 40);
    map.mutate(2,0);
    map.mutate(3,0);
    map.mutate(4,0);
    map.mutate(3,3);
    map.mutate(4,3);
    map.mutate(2,2);
    map.mutate(3,2);
    map.mutate(4,2);
    map.mutate(5,0);
    map.mutate(3,1);
    map.mutate(4,1);
    map.mutate(5,1);
    map.mutate(6,0);
    map.mutate(7,0);
    map.mutate(8,0);
    map.mutate(9,0);
    map.mutate(8,1);
    map.mutate(9,1);
    map.mutate(8,2);
    map.mutate(9,2);
    map.scan();
    config.allow_D = false;
    config.allow_x = false;
    config.allow_LR = false;
    TetrisMinoManager mino_manager("botris_srs.json");
    TetrisPathManager p_mgr(active, config, map);
    auto start = std::chrono::high_resolution_clock::now();
    auto result = p_mgr.test_run();
    auto end = std::chrono::high_resolution_clock::now();
    TetrisInstructor instructor(map, active.type);
    for (auto &active : result)
    {
        printf("%s\n", active.path.c_str());
        printf("x: %d, y: %d, r: %d\n", active.x, active.y, active.r);
        TetrisMap copy = map;
        copy.scan();
        instructor.attach(copy, active);
        for (int i = 7; i >= 0; i--)
        {
            printf("|");
            for (int j = 0; j < map.width; j++)
            {
                printf("%s", (copy.board[i] >> (j)) & 1 ? "[]" : "  ");
            }
            printf("|\n");
        }
    }
    printf("result size: %ld\n", result.size());
    printf("time: %ldns\n", std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}