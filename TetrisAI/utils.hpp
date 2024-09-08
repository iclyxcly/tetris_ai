#pragma once
#include <iostream>



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
