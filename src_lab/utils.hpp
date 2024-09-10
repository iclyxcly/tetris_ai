#pragma once
#include <chrono>
#include <thread>
#include "const.h"
namespace moenew
{
    inline void Sleep(int ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    inline constexpr bool within(const int t, const int min, const int max)
    {
        return t >= min && t < max;
    }
    inline constexpr Piece char_to_type(const char &c)
    {
        switch (c)
        {
        case 'S':
            return S;
        case 'L':
            return L;
        case 'Z':
            return Z;
        case 'I':
            return I;
        case 'T':
            return T;
        case 'O':
            return O;
        case 'J':
            return J;
        default:
            return X;
        }
    }
    inline constexpr char type_to_char(const Piece &p)
    {
        switch (p)
        {
        case S:
            return 'S';
        case L:
            return 'L';
        case Z:
            return 'Z';
        case I:
            return 'I';
        case T:
            return 'T';
        case O:
            return 'O';
        case J:
            return 'J';
        default:
            return 'X';
        }
    }
    inline constexpr char type_to_char(int &p)
    {
        switch (p)
        {
        case S:
            return 'S';
        case L:
            return 'L';
        case Z:
            return 'Z';
        case I:
            return 'I';
        case T:
            return 'T';
        case O:
            return 'O';
        case J:
            return 'J';
        default:
            return 'X';
        }
    }
    inline constexpr char type_to_char(const std::size_t &p)
    {
        switch (p)
        {
        case S:
            return 'S';
        case L:
            return 'L';
        case Z:
            return 'Z';
        case I:
            return 'I';
        case T:
            return 'T';
        case O:
            return 'O';
        case J:
            return 'J';
        default:
            return 'X';
        }
    }
}