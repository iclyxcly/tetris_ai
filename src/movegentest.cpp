#include "tetris_core.h"
#include <chrono>
int main()
{
    using namespace TetrisAI;
    TetrisConfig config;
    TetrisActive active(config.default_x, config.default_y, config.default_r, S);
    TetrisMap map(10, 40);
    map.mutate(1,1);
    map.mutate(3,0);
    TetrisMinoManager mino_manager("botris_srs.json");
    TetrisPathManager p_mgr(active, config, map, mino_manager.get_move_cache()[S], mino_manager.get()[S]);
    auto start = std::chrono::high_resolution_clock::now();
    p_mgr.test_run();
    auto end = std::chrono::high_resolution_clock::now();
    auto &result = p_mgr.result;
    for (auto &active : result)
    {
        printf("%s\n", active.path.c_str());
        printf("x: %d, y: %d, r: %d\n", active.x, active.y, active.r);
        for (int i = 3; i >= 0; i--)
        {
            printf("|");
            for (int j = 0; j < map.width; j++)
            {
                printf("%s", (active.snapshot[i] >> (j)) & 1 ? "[]" : "  ");
            }
            printf("|\n");
        }
    }
    printf("result size: %ld\n", result.size());
    printf("time: %ldns\n", std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
}