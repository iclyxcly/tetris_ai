#include "tetris_core.h"
#include "stdio.h"
#include <random>
#include "json.hpp"
#include <fstream>

int main()
{
    TetrisAI::TetrisMinoManager mino_manager("botris_srs.json");
    auto mino_list = mino_manager.get();
    TetrisAI::TetrisMap map(10, 40);
}