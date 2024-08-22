#include "tetris_core.h"
#include "stdio.h"
#include <random>
int main(void) {
    TetrisAI::TetrisMap map(10, 40);
    map.mutate(6, 0);
    map.mutate(5, 1);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::mt19937 mess_gen(rd());
    std::uniform_int_distribution<> dis(0, 9);
    std::uniform_int_distribution<> mess_dis(0, 100);
    std::queue<int8_t> pending;
    pending.push(3);
    pending.push(1);
    pending.push(6);
    pending.push(3);
    TetrisAI::TetrisPendingLineManager manager(pending, dis, mess_dis, gen, mess_gen);
    manager.take_all_damage(map, 0.05);
    for (int i = 20; i >= 0; i--) {
        printf("%2d |", i);
        for (int j = 0; j < 10; j++) {
            printf("%s", map.full(j, i) ? "[]" : "  ");
        }
        printf("\n");
    }
    return 0;
}