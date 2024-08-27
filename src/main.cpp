#include <iostream>
#include "utils.hpp"
#include "Botris_Client.hpp"
#include "tetris_core.h"
int main()
{
    ix::initNetSystem();
    {
        TetrisMinoManager mino_manager("botris_srs.json");
    }
    Botris_Client botris_client_p1;
    Botris_Client botris_client_p2;
    botris_client_p1.run();
    std::cin.get();
    return 0;
}
