#include "next.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>

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
    moenew::Next next;
    print_wait("Next object created, data: " + next.join());
    next.fill();
    do {
        print_wait("HOLD = " + moenew::type_to_string(next.hold) + ", NEXT = " + next.join());
        next.swap();
        print_wait("After swap, HOLD = " + moenew::type_to_string(next.hold) + ", NEXT = " + next.join());
        print_wait("POP = " + moenew::type_to_string(next.pop()) + ", NEXT = " + next.join());
        print_wait("PEEK = " + moenew::type_to_string(next.peek()) + ", NEXT = " + next.join());
        next.fill();
        print_wait("After fill, NEXT = " + next.join());
    } while(true);
    return 0;
}