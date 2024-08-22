#pragma once
#include <cstdint>
#include <cstring>
#include <queue>
#include <random>
#include <map>
#include <atomic>
#include <memory>
#include <fstream>
#include <string>
#include "json.hpp"

namespace TetrisAI
{
    using json = nlohmann::json;
    struct TetrisMap;
    struct TetrisConfig;
    struct TetrisCoord;
    struct TetrisActive;
    struct TetrisNextManager;
    struct TetrisPendingLineManager;
    struct TetrisStatus;
    struct TetrisAttackConfig;
    enum TetrisMinoType
    {
        S = 0,
        L = 1,
        Z = 2,
        I = 3,
        T = 4,
        O = 5,
        J = 6,
        EMPTY = 255
    };
    std::map<char, TetrisMinoType> char_to_type = {
        {'S', S},
        {'L', L},
        {'Z', Z},
        {'I', I},
        {'T', T},
        {'O', O},
        {'J', J},
        {' ', EMPTY}};
    constexpr uint8_t BOARD_HEIGHT = 40;
    constexpr uint32_t BINARY_TEMPLATE[33] = {
        0b1,
        0b10,
        0b100,
        0b1000,
        0b10000,
        0b100000,
        0b1000000,
        0b10000000,
        0b100000000,
        0b1000000000,
        0b10000000000,
        0b100000000000,
        0b1000000000000,
        0b10000000000000,
        0b100000000000000,
        0b1000000000000000,
        0b10000000000000000,
        0b100000000000000000,
        0b1000000000000000000,
        0b10000000000000000000,
        0b100000000000000000000,
        0b1000000000000000000000,
        0b10000000000000000000000,
        0b100000000000000000000000,
        0b1000000000000000000000000,
        0b10000000000000000000000000,
        0b100000000000000000000000000,
        0b1000000000000000000000000000,
        0b10000000000000000000000000000,
        0b100000000000000000000000000000,
        0b1000000000000000000000000000000,
        0b10000000000000000000000000000000};
    constexpr uint32_t CLEAR_REQUIREMENT[33] = {
        0b1,
        0b11,
        0b111,
        0b1111,
        0b11111,
        0b111111,
        0b1111111,
        0b11111111,
        0b111111111,
        0b1111111111,
        0b11111111111,
        0b111111111111,
        0b1111111111111,
        0b11111111111111,
        0b111111111111111,
        0b1111111111111111,
        0b11111111111111111,
        0b111111111111111111,
        0b1111111111111111111,
        0b11111111111111111111,
        0b111111111111111111111,
        0b1111111111111111111111,
        0b11111111111111111111111,
        0b111111111111111111111111,
        0b1111111111111111111111111,
        0b11111111111111111111111111,
        0b111111111111111111111111111,
        0b1111111111111111111111111111,
        0b11111111111111111111111111111,
        0b111111111111111111111111111111,
        0b1111111111111111111111111111111,
        0b11111111111111111111111111111111};
    struct TetrisMap
    {
        uint32_t board[BOARD_HEIGHT];
        uint32_t board_snapshot[4];
        uint8_t height;
        uint8_t width;
        uint8_t roof;
        TetrisMap(const uint8_t width, const uint8_t height) : height(height), width(width), roof(0)
        {
            memset(board, 0, sizeof(board));
            memset(board_snapshot, 0, sizeof(board_snapshot));
        }
        TetrisMap(const TetrisMap &other)
        {
            memcpy(board, other.board, sizeof(board));
            height = other.height;
            width = other.width;
            roof = other.roof;
        }
        void mutate(const uint8_t x, const uint8_t y)
        {
            board[y] ^= BINARY_TEMPLATE[x];
        }
        void push_cheese(const uint8_t &amount, const uint8_t &index)
        {
            memmove(board + amount, board, sizeof(board) - amount * sizeof(uint32_t));
            memset(board, 0, amount * sizeof(uint32_t));
            for (uint8_t i = 0; i < amount; i++)
            {
                uint32_t hole = 1 << index;
                board[i] = CLEAR_REQUIREMENT[width] ^ hole;
            }
        }
        bool full(const uint8_t &x, const uint8_t &y) const
        {
            return board[y] & BINARY_TEMPLATE[x];
        }
        void operator=(const TetrisMap &other)
        {
            memcpy(board, other.board, sizeof(board));
            height = other.height;
            width = other.width;
        }
        uint32_t operator[](const uint8_t index) const
        {
            return board[index];
        }
        uint8_t scan()
        {
            roof = 0;
            for (uint8_t i = height - 1; i >= 0; i--)
            {
                if (board[i] != 0)
                {
                    roof = ++i;
                    break;
                }
            }
            return roof;
        }
    };
    struct TetrisConfig
    {
        int8_t default_x;
        int8_t default_y;
        int8_t default_r;
        bool can_hold;
        bool allow_LR;
        bool allow_lr;
        bool allow_D;
        bool allow_d;
        bool allow_180;
        TetrisConfig() : default_x(3), default_y(20), default_r(0), can_hold(true), allow_LR(true), allow_lr(true), allow_D(true), allow_d(true), allow_180(true) {}
    };
    struct TetrisCoord
    {
        int8_t x;
        int8_t y;
        int8_t r;
        TetrisCoord() : x(0), y(0), r(0) {}
        TetrisCoord(int8_t x, int8_t y, int8_t r) : x(x), y(y), r(r) {}
    };
    struct TetrisActive : public TetrisCoord
    {
        uint8_t type;
        TetrisActive() : TetrisCoord(), type(EMPTY) {}
        TetrisActive(int8_t x, int8_t y, int8_t r, uint8_t type) : TetrisCoord(x, y, r), type(type) {}
    };
    struct TetrisNextManager
    {
        TetrisConfig &config;
        TetrisActive active;
        std::queue<uint8_t> queue;
        uint8_t hold;
        bool changed_hold;
        TetrisNextManager(TetrisConfig &config) : config(config), active(config.default_x, config.default_y, config.default_r, 0), hold(255), changed_hold(false) {}
        bool change_hold()
        {
            if (!config.can_hold || changed_hold)
                return false;

            if (hold == 255)
            {
                if (queue.empty())
                    return false;
                hold = active.type;
                active = TetrisActive(config.default_x, config.default_y, config.default_r, queue.front());
                queue.pop();
            }
            else
            {
                uint8_t temp = hold;
                hold = active.type;
                active = TetrisActive(config.default_x, config.default_y, config.default_r, temp);
            }

            changed_hold = true;
            return true;
        }
        bool next()
        {
            if (queue.empty())
                return false;
            active = TetrisActive(config.default_x, config.default_y, config.default_r, queue.front());
            queue.pop();
            changed_hold = false;
            return true;
        }
        void push(uint8_t type)
        {
            queue.push(type);
        }
        void insert(std::queue<uint8_t> &other)
        {
            while (!other.empty())
            {
                queue.push(other.front());
                other.pop();
            }
        }
        void replace(std::queue<uint8_t> &other)
        {
            std::swap(queue, other);
        }
    };
    struct TetrisPendingLineManager
    {
        // todo: random device
        std::uniform_int_distribution<> &dis;
        std::uniform_int_distribution<> &mess_dis;
        std::mt19937 gen;
        std::mt19937 mess_gen;
        std::queue<int8_t> pending;
        void fight_lines(int8_t &attack)
        {
            while (!pending.empty() && attack > 0)
            {
                if (pending.front() > attack)
                {
                    pending.front() -= attack;
                    attack = 0;
                }
                else
                {
                    attack -= pending.front();
                    pending.pop();
                }
            }
        }
        void take_all_damage(TetrisMap &map, double messiness)
        {
            while (!pending.empty())
            {
                int8_t line = pending.front();
                uint8_t index = dis(gen);
                pending.pop();
                for (int8_t i = 0; i < line; i++)
                {
                    if (mess_dis(mess_gen) < messiness * 100)
                    {
                        index = dis(gen);
                    }
                    map.push_cheese(1, index);
                }
            }
        }
        TetrisPendingLineManager(std::queue<int8_t> &pending, std::uniform_int_distribution<> &dis, std::uniform_int_distribution<> &mess_dis, std::mt19937 &gen, std::mt19937 &mess_gen) : dis(dis), mess_dis(mess_dis), gen(gen), mess_gen(mess_gen)
        {
            std::swap(this->pending, pending);
        }
        TetrisPendingLineManager(TetrisPendingLineManager &other) : dis(other.dis), mess_dis(other.mess_dis), gen(other.gen), mess_gen(other.mess_gen)
        {
            std::swap(pending, other.pending);
        }
    };
    struct TetrisStatus
    {
        int16_t b2b;
        int16_t combo;
        int8_t clear;
        int8_t spin_type;
        TetrisPendingLineManager garbage;
    };
    struct TetrisAttackConfig
    {
        double messiness = 0.0;
        int16_t b2b = 1;
        int16_t single = 0;
        int16_t double_ = 1;
        int16_t triple = 2;
        int16_t quad = 4;
        int16_t allspin_1 = 2;
        int16_t allspin_2 = 4;
        int16_t allspin_3 = 6;
        int16_t perfect_clear = 10;
        int16_t comboTable[12] = {0, 0, 1, 1, 1, 2, 2, 3, 3, 4, -1};
        int16_t get_combo_table(int16_t combo)
        {
            if (combo > 11)
                return comboTable[11];
            return comboTable[combo];
        }
    };
    using Wallkick = std::vector<std::vector<std::vector<std::pair<int8_t, int8_t>>>>;
    struct TetrisMino
    {
        uint8_t data[4][4];
        Wallkick rotate_right;
        Wallkick rotate_left;
        Wallkick rotate_180;
        int8_t up_offset[4];
        int8_t down_offset[4];
        int8_t left_offset[4];
        int8_t right_offset[4];
        // here
    };
    struct TetrisMinoManager
    {
        std::shared_ptr<std::map<TetrisMinoType, TetrisMino>> mino_list;
        std::shared_ptr<std::map<TetrisMinoType, TetrisMino>> get()
        {
            return mino_list;
        }
        TetrisMinoManager(std::string path)
        {
            json jsondata;
            std::ifstream file(path);
            file >> jsondata;
            file.close();
            mino_list = std::make_shared<std::map<TetrisMinoType, TetrisMino>>();
            try
            {
                jsondata = jsondata["minotypes"];
                for (int8_t i = 0; i < jsondata.size(); i++)
                {
                    TetrisMino mino;
                    // data
                    for (int8_t j = 0; j < 4; j++)
                    {
                        auto data = jsondata[i]["data"][j];
                        for (int8_t k = jsondata[i]["height"]; k >= 0; k--)
                        {
                            uint8_t row = 0;
                            for (int8_t l = 0; l < jsondata[i]["width"]; l++)
                            {
                                row |= (data[k][l] == 1) << l;
                            }
                            mino.data[j][k] = row;
                        }
                    }
                    // offset
                    for (int8_t j = 0; j < 4; j++)
                    {
                        auto data = jsondata[i]["offset"][j];
                        mino.up_offset[j] = data[0];
                        mino.right_offset[j] = data[1];
                        mino.down_offset[j] = data[2];
                        mino.left_offset[j] = data[3];
                    }
                    // clockwise_kicks
                    for (int8_t j = 0; j < 4; j++)
                    {
                        auto data = jsondata[i]["clockwise_kicks"][j];
                        std::vector<std::vector<std::pair<int8_t, int8_t>>> kicks;
                        for (int8_t k = 0; k < data.size(); k++)
                        {
                            std::vector<std::pair<int8_t, int8_t>> kick;
                            if (data[k].size() == 2)
                            {
                                kick.push_back(std::make_pair(data[k][0], data[k][1]));
                            }
                            else {
                                throw std::runtime_error("An error occurred while parsing the mino file, please check your file configuration.");
                            }
                            kicks.push_back(kick);
                        }
                        mino.rotate_right.push_back(kicks);
                    }
                    // counterclockwise_kicks
                    for (int8_t j = 0; j < 4; j++)
                    {
                        auto data = jsondata[i]["counter_clockwise_kicks"][j];
                        std::vector<std::vector<std::pair<int8_t, int8_t>>> kicks;
                        for (int8_t k = 0; k < data.size(); k++)
                        {
                            std::vector<std::pair<int8_t, int8_t>> kick;
                            if (data[k].size() == 2)
                            {
                                kick.push_back(std::make_pair(data[k][0], data[k][1]));
                            }
                            else {
                                throw std::runtime_error("An error occurred while parsing the mino file, please check your file configuration.");
                            }
                            kicks.push_back(kick);
                        }
                        mino.rotate_left.push_back(kicks);
                    }
                    // 180_kicks
                    for (int8_t j = 0; j < 4; j++)
                    {
                        auto data = jsondata[i]["180_kicks"][j];
                        std::vector<std::vector<std::pair<int8_t, int8_t>>> kicks;
                        for (int8_t k = 0; k < data.size(); k++)
                        {
                            std::vector<std::pair<int8_t, int8_t>> kick;
                            if (data[k].size() == 2)
                            {
                                kick.push_back(std::make_pair(data[k][0], data[k][1]));
                            }
                            else {
                                throw std::runtime_error("An error occurred while parsing the mino file, please check your file configuration.");
                            }
                            kicks.push_back(kick);
                        }
                        mino.rotate_180.push_back(kicks);
                    }
                    char char_to_type_index = jsondata[i]["type"].get<std::string>()[0];
                    mino_list->insert(std::make_pair(char_to_type[char_to_type_index], mino));
                }
            }
            catch (std::exception &e)
            {
                printf("%s\n", e.what());
                throw std::runtime_error("An error occurred while parsing the mino file, please check your file configuration.");
            }
        }
    };
}