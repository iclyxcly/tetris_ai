#include "pending.hpp"
#include "board.hpp"
#include "const.h"
#include <iostream>

void print_wait(std::string msg)
{
    std::cout << msg << std::endl;
    std::cin.get();
}

void print(std::string msg)
{
    std::cout << msg << std::endl;
}

int main()
{
    using namespace moenew;
    Pending pending;
    Board board;
    pending.push(2, 1);
    pending.push(3, 2);
    pending.push(4, 3);
    print("Pending total: " + std::to_string(pending.total()));
    print("Pending estimate: " + std::to_string(pending.estimate()));
    while (!pending.empty())
    {
        pending.accept(board, 0.05);
        print("Board status:\n" + board.print(20));
        print("Decaying...");
        pending.decay();
        print("Next pending total: " + std::to_string(pending.total()));
        print_wait("Next pending estimate: " + std::to_string(pending.estimate()));
    }
    return 0;
}