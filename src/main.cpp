#include <iostream>
#include "utils.hpp"
#include "Botris_Client.hpp"

int main() {    
    // ix::initNetSystem();
    
	Botris_Client botris_client_p1;
    Botris_Client botris_client_p2;
    botris_client_p1.run();
    botris_client_p2.run();
    std::cin.get();
    return 0;
}

