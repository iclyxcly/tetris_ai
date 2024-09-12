#include "tetris_core.h"
#include <chrono>
int main()
{
    using namespace TetrisAI;
    TetrisConfig config;
    TetrisActive active(config.default_x, config.default_y, config.default_r, S);
    TetrisMap map(10, 40);
    map.scan();
    TetrisMinoManager mino_manager("botris_srs.json");
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    int runs = 0;
    int total = 0;
    do {
        ++runs;
        TetrisPathManager p_mgr(active, config, map);
        p_mgr.test_run();
        total += p_mgr.result_db.size();
    } while (duration_cast<milliseconds>(high_resolution_clock::now() - start).count() < 1000);
    TetrisPathManager p_mgr(active, config, map);
    auto result = p_mgr.test_run();
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
    printf("runs: %d\n", runs);
    printf("total: %d\n", total);
}