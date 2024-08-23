#include "tetris_core.h"
#include <chrono>
int main()
{
    using namespace TetrisAI;
    TetrisConfig config;
    TetrisActive active(config.default_x, config.default_y, config.default_r, T);
    TetrisMap map(10, 40);
    TetrisMinoManager mino_manager("botris_srs.json");
    TetrisPathManager p_mgr(active, config, map, mino_manager.get_move_cache()[T], mino_manager.get()[T]);
    auto start = std::chrono::high_resolution_clock::now();
    p_mgr.run();
    auto end = std::chrono::high_resolution_clock::now();
    auto &result = p_mgr.result;
    for (auto &active : result)
    {
        printf("%s\n", active.path.c_str());
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