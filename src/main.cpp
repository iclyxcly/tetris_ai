#include <iostream>
#include "utils.hpp"
#include "Botris_Client.hpp"
int main()
{
    Botris_Client botris_client_p1(1);
    botris_client_p1.run();
    std::cin.get();
    return 0;
}
