#pragma once
#include <chrono>
#include <thread>
#include <string>
#include <cstddef>
#include <iostream>
#include "const.h"
namespace moenew
{
    inline void Sleep(int ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    inline constexpr bool within(const int t, const int min, const int max)
    {
        return t >= min && t <= max;
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
    inline constexpr char type_to_char(Piece p)
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
    inline const std::string type_to_string(const Piece &p)
    {
        switch (p)
        {
        case S:
            return "S";
        case L:
            return "L";
        case Z:
            return "Z";
        case I:
            return "I";
        case T:
            return "T";
        case O:
            return "O";
        case J:
            return "J";
        default:
            return "X";
        }
    }
    inline const std::string type_to_string(const int &p)
    {
        switch (p)
        {
        case S:
            return "S";
        case L:
            return "L";
        case Z:
            return "Z";
        case I:
            return "I";
        case T:
            return "T";
        case O:
            return "O";
        case J:
            return "J";
        default:
            return "X";
        }
    }
    inline const std::string type_to_string(const std::size_t &p)
    {
        switch (p)
        {
        case S:
            return "S";
        case L:
            return "L";
        case Z:
            return "Z";
        case I:
            return "I";
        case T:
            return "T";
        case O:
            return "O";
        case J:
            return "J";
        default:
            return "X";
        }
    }
}

namespace utils {
    enum LogLevel {
        DEBUG,
		INFO,
		WARN,
		ERR,
		FATAL
    };
    std::string get_level_color(int level) {
        switch (level) {
		case DEBUG:
			return "\033[1;34m[DEBUG]\033[0m";
		case INFO:
			return "\033[1;32m[INFO]\033[0m";
		case WARN:
			return "\033[1;33m[WARN]\033[0m";
		case ERR:
			return "\033[1;31m[ERROR]\033[0m";
		case FATAL:
			return "\033[1;35m[FATAL]\033[0m";
		default:
			return "";
        }
    }
    void print(int level, std::string s) {
		std::cout << get_level_color(level) << s << std::flush;
    }

    void printsp(std::string s) {
		std::cout << s << " " << std::flush;
    }

    void println(int level, std::string s) {
		std::cout << get_level_color(level) << s << "\n" << std::flush;
    }
}
