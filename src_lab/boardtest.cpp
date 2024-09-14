#include "board.hpp"
#include <iostream>
#include <bitset>
void print(std::string msg)
{
    std::cout << msg << std::endl;
}
void print_wait(std::string msg)
{
    std::cout << msg << std::endl;
    std::cin.get();
}
int main()
{
    using namespace moenew;
    Board board;
    print("Board object created, data: \n" + board.print(20) + "\ny_max: " + std::to_string(board.y_max) + ", cnt: " + std::to_string(board.cnt));
    std::string bit_str;
    for (int i = 0; i < 20; ++i)
    {
        bit_str += std::bitset<32>(board.board[i]).to_string() + "\n";
    }
    print_wait("Bit string: \n" + bit_str);
    bit_str.clear();
    for (int i = 0; i < 10; ++i)
    {
        board.set(i, i);
    }
    print("After setting (0,0), (1,1), ..., (9,9), data: \n" + board.print(20) + "\ny_max: " + std::to_string(board.y_max) + ", cnt: " + std::to_string(board.cnt));
    for (int i = 0; i < 20; ++i)
    {
        bit_str += std::bitset<32>(board.board[i]).to_string() + "\n";
    }
    print_wait("Bit string: \n" + bit_str);
    bit_str.clear();
    for (int i = 1; i < 10; ++i)
    {
        board.set(i, 0);
    }
    print("Trying to clear on this board: \n" + board.print(20) + "\ny_max: " + std::to_string(board.y_max) + ", cnt: " + std::to_string(board.cnt));
    for (int i = 0; i < 20; ++i)
    {
        bit_str += std::bitset<32>(board.board[i]).to_string() + "\n";
    }
    print_wait("Bit string: \n" + bit_str);
    bit_str.clear();
    int result = board.flush();
    print("Clear result: " + std::to_string(result) + ", data: \n" + board.print(20) + "\ny_max: " + std::to_string(board.y_max) + ", cnt: " + std::to_string(board.cnt));
    for (int i = 0; i < 20; ++i)
    {
        bit_str += std::bitset<32>(board.board[i]).to_string() + "\n";
    }
    print_wait("Bit string: \n" + bit_str);
    bit_str.clear();
    for (int i = 0; i < 10; ++i)
    {
        board.rise(1, i);
    }
    print("After rising (0,0), (1,1), ..., (9,9), data: \n" + board.print(20) + "\ny_max: " + std::to_string(board.y_max) + ", cnt: " + std::to_string(board.cnt));
    for (int i = 0; i < 20; ++i)
    {
        bit_str += std::bitset<32>(board.board[i]).to_string() + "\n";
    }
    print_wait("Bit string: \n" + bit_str);
    bit_str.clear();
    board.clear();
    print("After clearing, data: \n" + board.print(20) + "\ny_max: " + std::to_string(board.y_max) + ", cnt: " + std::to_string(board.cnt));
    for (int i = 0; i < 20; ++i)
    {
        bit_str += std::bitset<32>(board.board[i]).to_string() + "\n";
    }
    print_wait("Bit string: \n" + bit_str);
    bit_str.clear();
}