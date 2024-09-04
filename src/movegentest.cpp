#include "tetris_core.h"
#include <chrono>
int main()
{
    using namespace TetrisAI;
    TetrisConfig config;
    TetrisActive active(config.default_x, config.default_y, config.default_r, I);
    TetrisMap map(10, 40);
    map.mutate(0,0);
    map.mutate(1,0);
    map.mutate(2,0);
    map.mutate(3,0);
    map.mutate(4,0);
    map.mutate(3,1);
    map.mutate(4,1);
    map.mutate(5,0);
    map.scan();
    TetrisMinoManager mino_manager("botris_srs.json");
    TetrisPathManager p_mgr(active, config, map);
    auto start = std::chrono::high_resolution_clock::now();
    p_mgr.test_run();
    auto end = std::chrono::high_resolution_clock::now();
    auto &result = p_mgr.result;
    TetrisInstructor instructor(map, active.type);
    for (auto &active : result)
    {
        printf("%s\n", active.path.c_str());
        printf("x: %d, y: %d, r: %d\n", active.x, active.y, active.r);
        TetrisMap copy = map;
        copy.scan();
        instructor.attach(copy, active);
        for (int i = 3; i >= 0; i--)
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