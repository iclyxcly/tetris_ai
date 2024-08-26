#pragma once
#include <cstdint>
#include <random>
#include <map>
#include <unordered_set>
#include <fstream>
#include <string>
#include <queue>
#include <vector>
#include <memory>
#include <algorithm>
#include <limits>
#include <chrono>
#include "json.hpp"
#include <nmmintrin.h>
#define _mm_popcnt_u32 __builtin_popcount
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
    struct TetrisMino;
    struct TetrisMinoManager;
    struct TetrisInstructor;
    struct TetrisPathManager;
    struct TetrisNode;
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
    std::unordered_map<char, TetrisMinoType> char_to_type = {
        {'S', S},
        {'L', L},
        {'Z', Z},
        {'I', I},
        {'T', T},
        {'O', O},
        {'J', J},
        {' ', EMPTY}};
    constexpr uint8_t BOARD_HEIGHT = 40;
    constexpr uint32_t BINARY_TEMPLATE[32] = {
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
    constexpr uint32_t CLEAR_REQUIREMENT[32] = {
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
        uint8_t height;
        uint8_t width;
        uint8_t roof;
        TetrisMap(const uint8_t width, const uint8_t height) : height(height), width(width), roof(0)
        {
            memset(board, 0, sizeof(board));
        }
        TetrisMap(const TetrisMap &other)
        {
            memcpy(board, other.board, sizeof(board));
            height = other.height;
            width = other.width;
            roof = other.roof;
        }
        TetrisMap() : height(40), width(10), roof(0)
        {
            memset(board, 0, sizeof(board));
        }
        void mutate(const uint8_t x, const uint8_t y)
        {
            board[y] ^= BINARY_TEMPLATE[x];
        }
        void push_cheese(const uint8_t &amount, const uint8_t &index)
        {
            memmove(board + amount, board, sizeof(board) - amount * sizeof(uint32_t));
            memset(board, 0, amount * sizeof(uint32_t));
            uint32_t hole = 0;
            for (uint8_t i = 0; i < amount; i++)
            {
                hole = 1 << index;
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
            roof = other.roof;
        }
        uint32_t operator[](const uint8_t &index) const
        {
            return board[index];
        }
        uint8_t scan()
        {
            roof = height - 1;
            while (board[roof - 1] == 0 && roof > 0)
            {
                roof--;
            };
            return roof;
        }
        uint8_t flush()
        {
            uint8_t clear = 0;
            const auto &req = CLEAR_REQUIREMENT[width - 1];
            for (int8_t i = height - 1; i >= 0; i--)
            {
                if (board[i] == req)
                {
                    std::memmove(&board[i], &board[i + 1], sizeof(uint32_t) * (height - i));
                    board[height - 1] = 0;
                    clear++;
                }
            }
            return clear;
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
        std::size_t beam_width;
        time_t target_time;
        TetrisConfig() : default_x(3), default_y(18), default_r(0), can_hold(true), allow_LR(true), allow_lr(true), allow_D(true), allow_d(true), allow_180(false), beam_width(8), target_time(100) {}
    };
    struct TetrisCoord
    {
        int8_t x;
        int8_t y;
        int8_t r;
        TetrisCoord() : x(0), y(0), r(0) {}
        TetrisCoord(const int8_t &x, const int8_t &y, const int8_t &r) : x(x), y(y), r(r) {}
        TetrisCoord(const TetrisConfig &config) : x(config.default_x), y(config.default_y), r(config.default_r) {}
        bool operator==(const TetrisCoord &other) const
        {
            return x == other.x && y == other.y && r == other.r;
        }
    };
    struct TetrisCoordHash
    {
        int operator()(const TetrisCoord &coord) const
        {
            int combined = coord.x * 31 + coord.y;
            combined = combined * 31 + coord.r;

            return combined;
        }
    };

    struct TetrisActive : public TetrisCoord
    {
        uint8_t type;
        bool last_rotate;
        int8_t last_kick;
        std::string path;
        uint32_t snapshot[BOARD_HEIGHT];
        bool operator==(const uint32_t other[4]) const
        {
            return std::memcmp(snapshot, other, sizeof(snapshot)) == 0;
        }
        bool operator==(const TetrisActive &other) const
        {
            return x == other.x && y == other.y && r == other.r;
        }
        void operator=(const TetrisActive &other)
        {
            x = other.x;
            y = other.y;
            r = other.r;
            type = other.type;
            last_rotate = other.last_rotate;
            last_kick = other.last_kick;
            path = other.path;
            std::memcpy(snapshot, other.snapshot, sizeof(snapshot));
        }
        TetrisActive() : TetrisCoord(), type(EMPTY), last_rotate(false), last_kick(-1) {}
        TetrisActive(const int8_t &x, const int8_t &y, const int8_t &r, const uint8_t &type) : TetrisCoord(x, y, r), type(type), last_rotate(false), last_kick(-1) {}
        TetrisActive(const TetrisActive &other) : TetrisCoord(other.x, other.y, other.r), type(other.type), last_rotate(other.last_rotate), last_kick(other.last_kick), path(other.path)
        {
            std::memcpy(snapshot, other.snapshot, sizeof(snapshot));
        }
    };
    struct TetrisNextManager
    {
        TetrisConfig &config;
        TetrisActive active;
        std::queue<uint8_t> queue;
        uint8_t hold;
        bool changed_hold;
        TetrisNextManager(TetrisConfig &config) : config(config), active(config.default_x, config.default_y, config.default_r, 0), hold(255), changed_hold(false) {}
        TetrisNextManager(TetrisNextManager &other) : config(other.config), active(other.active), hold(other.hold), changed_hold(other.changed_hold)
        {
            queue = other.queue;
        }
        void operator=(TetrisNextManager &other)
        {
            config = other.config;
            active = other.active;
            hold = other.hold;
            changed_hold = other.changed_hold;
            queue = other.queue;
        }
        bool change_hold()
        {
            if (!config.can_hold || changed_hold)
                return false;

            if (hold == EMPTY)
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
            active.path += 'v';
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
            queue = other;
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
        void fight_lines(int &attack)
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
        void push_lines(int8_t line)
        {
            pending.push(line);
        }
        TetrisPendingLineManager(std::queue<int8_t> &pending, std::uniform_int_distribution<> &dis, std::uniform_int_distribution<> &mess_dis, std::mt19937 &gen, std::mt19937 &mess_gen) : dis(dis), mess_dis(mess_dis), gen(gen), mess_gen(mess_gen)
        {
            std::swap(this->pending, pending);
        }
        TetrisPendingLineManager(TetrisPendingLineManager &other) : dis(other.dis), mess_dis(other.mess_dis), gen(other.gen), mess_gen(other.mess_gen)
        {
            std::swap(pending, other.pending);
        }
        TetrisPendingLineManager(std::uniform_int_distribution<> &dis, std::uniform_int_distribution<> &mess_dis, std::mt19937 &gen, std::mt19937 &mess_gen) : dis(dis), mess_dis(mess_dis), gen(gen), mess_gen(mess_gen) {}
    };
    struct TetrisStatus
    {
        int16_t b2b;
        int16_t combo;
        int8_t clear;
        int8_t spin_type;
        TetrisNextManager next;
        TetrisPendingLineManager garbage;
        TetrisStatus(int16_t b2b, int16_t combo, TetrisNextManager &next, TetrisPendingLineManager &garbage) : b2b(b2b), combo(combo), clear(0), spin_type(0), next(next), garbage(garbage) {}
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
    using TetrisWallkick = std::vector<std::vector<std::pair<int8_t, int8_t>>>;
    struct TetrisMino
    {
        uint8_t data[4][4];
        TetrisWallkick rotate_right;
        TetrisWallkick rotate_left;
        TetrisWallkick rotate_180;
        int8_t up_offset[4];
        int8_t down_offset[4];
        int8_t left_offset[4];
        int8_t right_offset[4];
    };
    using TetrisMinotypes = std::unordered_map<uint8_t, TetrisMino>;
    using TetrisMinocache = std::unordered_map<uint8_t, std::vector<std::unordered_map<int8_t, uint32_t[4]>>>;
    using TetrisMinocacheMini = std::vector<std::unordered_map<int8_t, uint32_t[4]>>;
    struct TetrisMinoManager
    {
        static TetrisMinotypes mino_list;
        static TetrisMinocache move_cache;
        bool loaded = false;
        static TetrisMinotypes &get()
        {
            return mino_list;
        }
        static TetrisMinocache &get_move_cache()
        {
            return move_cache;
        }
        TetrisMinoManager(const std::string &path)
        {
            if (mino_list.size() > 0)
                return;

            json jsondata;
            std::ifstream file(path);
            file >> jsondata;
            file.close();
            try
            {
                jsondata = jsondata["minotypes"];
                for (std::size_t i = 0; i < jsondata.size(); i++)
                {
                    TetrisMino mino;
                    // data
                    for (int8_t j = 0; j < 4; j++)
                    {
                        auto data = jsondata[i]["data"][j];
                        memset(mino.data[j], 0, sizeof(mino.data[j]));
                        for (int8_t k = 0; k < jsondata[i]["height"]; k++)
                        {
                            uint8_t row = 0;
                            for (int8_t l = 0; l < jsondata[i]["width"]; l++)
                            {
                                row |= (data[k][l] == 1) << l;
                            }
                            mino.data[j][3 - k] = row;
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
                        json data = jsondata[i]["clockwise_kicks"][j];
                        std::vector<std::pair<int8_t, int8_t>> kick;
                        for (std::size_t k = 0; k < data.size(); k++)
                        {
                            if (data[k].size() == 2)
                            {
                                kick.push_back(std::make_pair(data[k][0], data[k][1]));
                            }
                            else
                            {
                                throw std::runtime_error("clockwise kicktest missing xy");
                            }
                        }
                        mino.rotate_right.push_back(kick);
                    }
                    // counterclockwise_kicks
                    for (int8_t j = 0; j < 4; j++)
                    {
                        auto data = jsondata[i]["counter_clockwise_kicks"][j];
                        std::vector<std::pair<int8_t, int8_t>> kick;
                        for (std::size_t k = 0; k < data.size(); k++)
                        {
                            if (data[k].size() == 2)
                            {
                                kick.push_back(std::make_pair(data[k][0], data[k][1]));
                            }
                            else
                            {
                                throw std::runtime_error("counter clockwise kicktest missing xy");
                            }
                        }
                        mino.rotate_left.push_back(kick);
                    }
                    // 180_kicks
                    for (int8_t j = 0; j < 4; j++)
                    {
                        auto data = jsondata[i]["180_kicks"][j];
                        std::vector<std::pair<int8_t, int8_t>> kick;
                        for (std::size_t k = 0; k < data.size(); k++)
                        {
                            if (data[k].size() == 2)
                            {
                                kick.push_back(std::make_pair(data[k][0], data[k][1]));
                            }
                            else
                            {
                                throw std::runtime_error("180 kicktest missing xy");
                            }
                        }
                        mino.rotate_180.push_back(kick);
                    }
                    char type = jsondata[i]["type"].get<std::string>()[0];
                    mino_list.insert(std::make_pair(char_to_type[type], mino));
                    for (int8_t i = 0; i < 4; ++i)
                    {
                        std::unordered_map<int8_t, uint32_t[4]> moves;
                        int8_t offset_right = 28 - mino.right_offset[i];
                        for (int8_t j = mino.left_offset[i]; j <= offset_right; ++j)
                        {
                            for (int8_t k = 0; k < 4; ++k)
                            {
                                if (j < 0)
                                {
                                    moves[j][k] = mino.data[i][k] >> -j;
                                }
                                else
                                {
                                    moves[j][k] = mino.data[i][k] << j;
                                }
                            }
                        }
                        move_cache[char_to_type[type]].push_back(moves);
                    }
                }
                loaded = true;
            }
            catch (std::exception &e)
            {
                printf("%s\n", e.what());
                throw std::runtime_error("An error occurred while parsing the mino file, please check your file configuration.");
            }
        }
    };
    TetrisAI::TetrisMinotypes TetrisAI::TetrisMinoManager::mino_list;
    TetrisAI::TetrisMinocache TetrisAI::TetrisMinoManager::move_cache;
    struct TetrisAttack
    {
        double messiness;
        uint8_t single = 0;
        uint8_t double_ = 1;
        uint8_t triple = 2;
        uint8_t quad = 4;
        uint8_t asd = 4;
        uint8_t ass = 2;
        uint8_t ast = 6;
        uint8_t pc = 10;
        uint8_t b2b = 1;
        double multiplier = 0;
        uint8_t combo_table[20] = {0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 4, 4, 4, 4, 4};
    } atk;
    struct TetrisParam
    {
        double roof = 40;
        double col_trans = 15;
        double row_trans = 20;
        double aggregate_height = 1;
        double bumpiness = 0;
        double hole_count = 24;
        double hole_line = 15;
        double b2b = 320;
        double attack = 40;
        double clear_1 = -100;
        double clear_2 = -20;
        double clear_3 = -20;
        double clear_4 = 20;
        double aspin_1 = 100;
        double aspin_2 = 100;
        double aspin_3 = 100;
        double aspin_slot = 10;
        double combo = 20;
        double acc_rating_rate = 0.2;
    } p;
    struct TetrisJudge
    {
        struct TetrisEvalTemplate
        {
            int32_t col_trans = 0;
            int32_t row_trans = 0;
            int32_t aggregate_height = 0;
            int16_t bumpiness = 0;
            int16_t hole_count = 0;
            int8_t hole_line = 0;
            int32_t spin_slot = 0;
        };
        static void find_s_spin(const TetrisMap &map, int32_t &val)
        { // no problem
            const uint32_t *ref = BINARY_TEMPLATE;
            for (int y = map.roof; y >= std::max<int>(0, map.roof - 3); --y)
            {
                uint32_t rowm1 = map.board[y - 1];
                uint32_t row0 = map.board[y];
                uint32_t row1 = map.board[y + 1];
                uint32_t row2 = map.board[y + 2];
                uint32_t row3 = map.board[y + 3];
                uint8_t count0 = _mm_popcnt_u32(map.board[y]);
                uint8_t count1 = _mm_popcnt_u32(map.board[y + 1]);
                uint8_t count2 = _mm_popcnt_u32(map.board[y + 2]);
                for (int x = 0; x < map.width - 2; ++x)
                {
                    if (~row0 & ref[x] && ~row0 & ref[x + 1] && ~row1 & ref[x + 1] && ~row1 & ref[x + 2])
                    {
                        // double/single
                        if ((row0 & ref[x + 2] || (y == 0 || rowm1 & ref[x + 1])) && (row1 & ref[x] || (row2 & ref[x + 2] && (x == 0 || row0 & ref[x - 1]))))
                        {
                            ++val;
                            if (count0 == 8)
                            {
                                val += 2;
                            }
                            if (count1 == 8)
                            {
                                val += 2;
                            }
                        }
                    }
                    if (~row2 & ref[x] && ~row1 & ref[x] && ~row1 & ref[x + 1] && ~row0 & ref[x + 1])
                    {
                        // triple
                        if (row0 & ref[x] && (row2 & ref[x + 1] || (row0 & ref[x + 2] && row3 & ref[x])))
                        {
                            ++val;
                            if (count0 == 9)
                            {
                                val += 2;
                            }
                            if (count1 == 2)
                            {
                                val += 2;
                            }
                            if (count2 == 9)
                            {
                                val += 2;
                            }
                        }
                    }
                }
            }
        }
        static void find_z_spin(const TetrisMap &map, int32_t &val)
        { // no problem
            const uint32_t *ref = BINARY_TEMPLATE;
            for (int y = map.roof; y >= std::max<int>(0, map.roof - 3); --y)
            {
                uint32_t rowm1 = map.board[y - 1];
                uint32_t row0 = map.board[y];
                uint32_t row1 = map.board[y + 1];
                uint32_t row2 = map.board[y + 2];
                uint32_t row3 = map.board[y + 3];
                uint8_t count0 = _mm_popcnt_u32(map.board[y]);
                uint8_t count1 = _mm_popcnt_u32(map.board[y + 1]);
                uint8_t count2 = _mm_popcnt_u32(map.board[y + 2]);
                for (int x = 0; x < map.width - 2; ++x)
                {
                    // double/single
                    if (~row1 & ref[x] && ~row1 & ref[x + 1] && ~row0 & ref[x + 1] && ~row0 & ref[x + 2])
                    {
                        if ((row0 & ref[x] || (y == 0 || rowm1 & ref[x])) && (row1 & ref[x + 2] || (row2 & ref[x] && (x + 3 == map.width || row0 & ref[x + 3]))))
                        {
                            ++val;
                            if (count0 == 8)
                            {
                                val += 2;
                            }
                            if (count1 == 8)
                            {
                                val += 2;
                            }
                        }
                    }
                    if (~row2 & ref[x + 1] && ~row1 & ref[x + 1] && ~row1 & ref[x] && ~row0 & ref[x])
                    {
                        // triple
                        if (row0 & ref[x + 1] && (row2 & ref[x] || ((x != 0 && row0 & ref[x - 1]) && row3 & ref[x + 1])))
                        {
                            ++val;
                            if (count0 == 9)
                            {
                                val += 2;
                            }
                            if (count1 == 2)
                            {
                                val += 2;
                            }
                            if (count2 == 9)
                            {
                                val += 2;
                            }
                        }
                    }
                }
            }
        }
        static void find_l_spin(const TetrisMap &map, int32_t &val)
        { // no problem
            const uint32_t *ref = BINARY_TEMPLATE;
            for (int y = map.roof; y >= std::max<int>(0, map.roof - 4); --y)
            {
                uint32_t rowm1 = map.board[y - 1];
                uint32_t row0 = map.board[y];
                uint32_t row1 = map.board[y + 1];
                uint32_t row2 = map.board[y + 2];
                uint32_t row3 = map.board[y + 3];
                uint8_t count0 = _mm_popcnt_u32(map.board[y]);
                uint8_t count1 = _mm_popcnt_u32(map.board[y + 1]);
                uint8_t count2 = _mm_popcnt_u32(map.board[y + 2]);
                for (int x = 0; x < map.width - 2; ++x)
                {
                    if (~row0 & ref[x] && ~row0 & ref[x + 1] && ~row0 & ref[x + 2] && ~row1 & ref[x + 2])
                    {
                        // double
                        bool cond1 = row1 & ref[x + 1] && (x + 3 == map.width || row0 & ref[x + 3] || row1 & ref[x + 3]);
                        bool cond2 = (x + 3 == map.width || row1 & ref[x + 3]) && row1 & ref[x];
                        bool cond3 = y == 0 || rowm1 & ref[x] || rowm1 & ref[x + 1] || rowm1 & ref[x + 2];
                        if (cond1 && cond2 && cond3)
                        {
                            ++val;
                            if (count0 == 7)
                            {
                                val += 2;
                            }
                            if (count1 == 9)
                            {
                                val += 2;
                            }
                        }
                    }
                    if (~row0 & ref[x] && ~row1 & ref[x] && ~row1 & ref[x + 1] && ~row1 & ref[x + 2])
                    {
                        // double, rotate 2x
                        bool cond1 = (x == 0 || row1 & ref[x - 1]) && row0 & ref[x + 1] && row2 & ref[x + 2];
                        bool cond2 = x != 0 && row0 & ref[x - 1] && row0 & ref[x + 1] && row2 & ref[x + 1];
                        bool cond3 = (x == 0 || row1 & ref[x - 1] || row0 & ref[x - 1]) && row0 & ref[x + 1] && row2 & ref[x + 1];
                        if (cond1 || cond2 || cond3)
                        {
                            if (count0 == 9)
                            {
                                val += 2;
                            }
                            if (count1 == 7)
                            {
                                val += 2;
                            }
                        }
                        if (cond1)
                        {
                            ++val;
                        }
                        if (cond2)
                        {
                            ++val;
                        }
                        if (cond3)
                        {
                            ++val;
                        }
                    }
                    if (~row0 & ref[x] && ~row1 & ref[x] && ~row2 & ref[x] && ~row0 & ref[x + 1])
                    {
                        // triple
                        bool cond1 = x == 0 || row0 & ref[x - 1] || row1 & ref[x - 1] || row2 & ref[x - 1];
                        bool cond2 = row1 & ref[x + 1] || (row3 & ref[x] && row0 * ref[x + 2]);
                        bool cond3 = y == 0 || rowm1 & ref[x] || rowm1 & ref[x + 1];
                        if (cond1 && cond2 && cond3)
                        {
                            ++val;
                            if (count0 == 8)
                            {
                                val += 2;
                            }
                            if (count1 == 9)
                            {
                                val += 2;
                            }
                            if (count2 == 9)
                            {
                                val += 2;
                            }
                        }
                    }
                }
            }
        }
        static void find_j_spin(const TetrisMap &map, int32_t &val)
        { // no problem
            const uint32_t *ref = BINARY_TEMPLATE;
            for (int y = map.roof; y >= std::max<int>(0, map.roof - 4); --y)
            {
                uint32_t rowm1 = map.board[y - 1];
                uint32_t row0 = map.board[y];
                uint32_t row1 = map.board[y + 1];
                uint32_t row2 = map.board[y + 2];
                uint8_t count0 = _mm_popcnt_u32(map.board[y]);
                uint8_t count1 = _mm_popcnt_u32(map.board[y + 1]);
                uint8_t count2 = _mm_popcnt_u32(map.board[y + 2]);
                for (int x = 0; x < map.width - 2; ++x)
                {
                    if (~row0 & ref[x] && ~row0 & ref[x + 1] && ~row0 & ref[x + 2] && ~row1 & ref[x])
                    {
                        // double
                        bool cond1 = row1 & ref[x + 1] && (x == 0 || row0 & ref[x - 1] || row1 & ref[x - 1]);
                        bool cond2 = (x == 0 || row1 & ref[x - 1]) && row1 & ref[x + 2];
                        bool cond3 = y == 0 || rowm1 & ref[x] || rowm1 & ref[x + 1] || rowm1 & ref[x + 2];
                        if (cond1 && cond2 && cond3)
                        {
                            ++val;
                            if (count0 == 7)
                            {
                                val += 2;
                            }
                            if (count1 == 9)
                            {
                                val += 2;
                            }
                        }
                    }
                    if (~row1 & ref[x] && ~row1 & ref[x + 1] && ~row1 & ref[x + 2] && ~row0 & ref[x + 2])
                    {
                        // double, rotate 2x
                        bool cond1 = (x + 3 == map.width || row1 & ref[x + 3]) && row0 & ref[x + 1] && row2 & ref[x];
                        bool cond2 = x + 3 != map.width && row0 & ref[x + 3] && row0 & ref[x + 1] && row2 & ref[x + 1];
                        bool cond3 = (x + 3 == map.width || row1 & ref[x + 3] || row0 & ref[x + 3]) && row0 & ref[x + 1] && row2 & ref[x + 1];
                        if (cond1 || cond2 || cond3)
                        {
                            if (count0 == 9)
                            {
                                val += 2;
                            }
                            if (count1 == 7)
                            {
                                val += 2;
                            }
                        }
                        if (cond1)
                        {
                            ++val;
                        }
                        if (cond2)
                        {
                            ++val;
                        }
                        if (cond3)
                        {
                            ++val;
                        }
                    }
                    if (~row0 & ref[x] && ~row0 & ref[x + 1] && ~row1 & ref[x + 1] && ~row2 & ref[x + 1])
                    {
                        // triple
                        bool cond1 = row0 & ref[x + 2] || row1 & ref[x + 2] || row2 & ref[x + 2];
                        bool cond2 = row1 & ref[x];
                        bool cond3 = y == 0 || rowm1 & ref[x] || rowm1 & ref[x + 1];
                        if (cond1 && cond2 && cond3)
                        {
                            ++val;
                            if (count0 == 8)
                            {
                                val += 2;
                            }
                            if (count1 == 9)
                            {
                                val += 2;
                            }
                            if (count2 == 9)
                            {
                                val += 2;
                            }
                        }
                    }
                }
            }
        }
        static void find_t_spin(const TetrisMap &map, int32_t &val)
        { // no problem
            const uint32_t *ref = BINARY_TEMPLATE;
            for (int y = map.roof; y >= std::max<int>(0, map.roof - 4); --y)
            {
                uint32_t rowm1 = map.board[y - 1];
                uint32_t row0 = map.board[y];
                uint32_t row1 = map.board[y + 1];
                uint32_t row2 = map.board[y + 2];
                uint8_t count0 = _mm_popcnt_u32(map.board[y]);
                uint8_t count1 = _mm_popcnt_u32(map.board[y + 1]);
                uint8_t count2 = _mm_popcnt_u32(map.board[y + 2]);
                for (int x = 0; x < map.width - 2; ++x)
                {
                    if (~row0 & ref[x + 1] && ~row1 & ref[x] && ~row1 & ref[x + 1] && ~row1 & ref[x + 2] && ~row2 & ref[x + 1])
                    {
                        // double
                        if (row0 & ref[x] && row0 & ref[x + 2] && (row2 & ref[x] || row2 & ref[x + 2]))
                        {
                            ++val;
                            if (count0 == 9)
                            {
                                val += 2;
                            }
                            if (count1 == 7)
                            {
                                val += 2;
                            }
                            if (count2 == 9)
                            { // imperial cross?
                                val += 2;
                            }
                        }
                    }
                    if (~row0 & ref[x + 1] && ~row1 & ref[x + 1] && ~row2 & ref[x + 1])
                    {
                        if ((~row1 & ref[x] && row1 & ref[x + 2]) || (row1 & ref[x] && ~row1 & ref[x + 2]))
                        {
                            // triple
                            if (row0 & ref[x] && row0 & ref[x + 2] && row2 & ref[x] && row2 & ref[x + 2])
                            {
                                ++val;
                                if (count0 == 9)
                                {
                                    val += 2;
                                }
                                if (count1 == 8)
                                {
                                    val += 2;
                                }
                                if (count2 == 9)
                                {
                                    val += 2;
                                }
                            }
                        }
                    }
                    if (~row0 & ref[x] && ~row0 & ref[x + 1] && ~row0 & ref[x + 2] && ~row1 & ref[x + 1])
                    {
                        // single (mini)
                        bool cond1 = row1 & ref[x] && (x + 3 == map.width || row0 & ref[x + 3]);
                        bool cond2 = row1 & ref[x + 2] && (x == 0 || row0 & ref[x - 1]);
                        bool cond3 = y == 0 || rowm1 & ref[x] || rowm1 & ref[x + 1] || rowm1 & ref[x + 2];
                        if ((cond1 || cond2) && cond3)
                        {
                            if (count0 == 7)
                            {
                                val += 2;
                            }
                        }
                        if (cond1)
                        {
                            ++val;
                        }
                        if (cond2)
                        {
                            ++val;
                        }
                    }
                }
            }
        }
        static void find_i_spin(const TetrisMap &map, int32_t &val)
        { // no problem
            const uint32_t *ref = BINARY_TEMPLATE;
            for (int y = map.roof; y >= std::max<int>(0, map.roof - 4); --y)
            {
                uint32_t rowm1 = map.board[y - 1];
                uint32_t row0 = map.board[y];
                uint32_t row1 = map.board[y + 1];
                uint32_t row2 = map.board[y + 2];
                uint32_t row3 = map.board[y + 3];
                uint32_t row4 = map.board[y + 4];
                uint8_t count0 = _mm_popcnt_u32(map.board[y]);
                uint8_t count1 = _mm_popcnt_u32(map.board[y + 1]);
                uint8_t count2 = _mm_popcnt_u32(map.board[y + 2]);
                for (int x = 0; x < map.width; ++x)
                {
                    if (x < map.width - 3 && ~row0 & ref[x] && ~row0 & ref[x + 1] && ~row0 & ref[x + 2] && ~row0 & ref[x + 3])
                    {
                        // single
                        bool up_cover = row1 & ref[x] || row1 & ref[x + 1] || row1 & ref[x + 2] || row1 & ref[x + 3];
                        if ((x == 0 || row0 & ref[x - 1]) && (x + 4 == map.width || row0 & ref[x + 4]) && up_cover)
                        {
                            ++val;
                            if (count0 == 6)
                            {
                                val += 2;
                            }
                        }
                    }
                    if (~row0 & ref[x] && ~row1 & ref[x] && ~row2 & ref[x] && ~row3 & ref[x])
                    {
                        // triple
                        bool left_cover = x == 0 || row0 & ref[x - 1] || row1 & ref[x - 1] || row2 & ref[x - 1] || row3 & ref[x - 1];
                        bool right_cover = x + 1 == map.width || row0 & ref[x + 1] || row1 & ref[x + 1] || row2 & ref[x + 1] || row3 & ref[x + 1];
                        if (row4 & ref[x] && left_cover && right_cover && (y == 0 || rowm1 & ref[x]))
                        {
                            ++val;
                            if (count0 == 9)
                            {
                                val += 2;
                            }
                            if (count1 == 9)
                            {
                                val += 2;
                            }
                            if (count2 == 9)
                            {
                                val += 2;
                            }
                        }
                    }
                }
            }
        }
        static void find_allspin(const TetrisMap &map, int32_t &v)
        {

            // std::string output = "";
            // for (int i = 24; i >= 0; i--) {
            //	bool minoAreaY = i >= path.coord.y && i < path.coord.y + 4;
            //	char buffer[256] = { '\0' };
            //	sprintf(buffer, "%2d|", i);
            //	output += buffer;
            //	for (int j = 0; j < map.width; j++) {
            //		bool minoAreaX = j >= path.coord.x && j < path.coord.x + 4;
            //		/*bool minoPixel = minoAreaX && minoAreaY && test.mino_full(Minodata::MINOTYPES[block.coord.t][block.coord.r], j - block.coord.x, i - block.coord.y);*/
            //		bool minoPixel = minoAreaX && minoAreaY && (Minodata::MINOTYPES[path.coord.t][path.coord.r][3 - (i - path.coord.y)] & (8 >> (j - path.coord.x)));
            //		output += (map.full(j, i)) || minoPixel ? minoPixel ? "()" : "[]" : "  ";
            //	}
            //	output += "|\n";
            // }
            // printf("%s", output.c_str());
            find_s_spin(map, v);
            find_z_spin(map, v);
            find_l_spin(map, v);
            find_j_spin(map, v);
            find_t_spin(map, v);
            find_i_spin(map, v);
        }
        static double begin_judgement(const TetrisStatus &last, TetrisStatus &now, TetrisMap &map, const bool &dead)
        {
            TetrisEvalTemplate eval;
            int aggregate_height[32];
            memset(aggregate_height, 0, sizeof(aggregate_height));
            for (int i = 0; i < map.roof; ++i)
            {
                for (int j = 0; j < map.width; j++)
                {
                    if (map.full(j, i))
                    {
                        aggregate_height[j] = i + 1;
                    }
                    if (j + 1 != map.width)
                    {
                        eval.row_trans += map.full(j, i) != map.full(j + 1, i);
                    }
                }
                if (i + 1 < map.roof)
                {
                    int check = eval.hole_count;
                    eval.hole_count += _mm_popcnt_u32(~map.board[i] & map.board[i + 1]);
                    eval.hole_line += check != eval.hole_count;
                    eval.col_trans += _mm_popcnt_u32(map.board[i] ^ map.board[i + 1]);
                }
            }
            find_allspin(map, eval.spin_slot);
            for (int i = 0; i < map.width; i++)
            {
                eval.aggregate_height += aggregate_height[i];
                if (i != 0)
                {
                    eval.bumpiness += std::abs(aggregate_height[i - 1] - aggregate_height[i]);
                }
            }
            double like = 0;
            int attack = 0;
            switch (now.clear)
            {
            case 0:
                now.combo = 0;
                if (now.garbage.pending.size() > 0)
                {
                    now.garbage.take_all_damage(map, atk.messiness);
                }
                break;
            case 1:
                if (now.spin_type == 3)
                {
                    attack += atk.ass + now.b2b;
                    like += p.aspin_1;
                    now.b2b = 1;
                }
                else
                {
                    like += p.clear_1;
                    attack += atk.single;
                    now.b2b = 0;
                }
                attack += atk.combo_table[now.combo++];
                break;
            case 2:
                if (now.spin_type == 3)
                {
                    attack += atk.asd + now.b2b;
                    like += p.aspin_2;
                    now.b2b = 1;
                }
                else
                {
                    like += p.clear_2;
                    attack += atk.double_;
                    now.b2b = 0;
                }
                attack += atk.combo_table[now.combo++];
                break;
            case 3:
                if (now.spin_type == 3)
                {
                    attack += atk.ast + now.b2b;
                    like += p.aspin_3;
                    now.b2b = 1;
                }
                else
                {
                    like += p.clear_3;
                    attack += atk.triple;
                    now.b2b = 0;
                }
                attack += atk.combo_table[now.combo++];
                break;
            case 4:
                like += p.clear_4;
                attack += atk.quad + now.b2b + atk.combo_table[now.combo++];
            }
            if (!map.roof)
            {
                attack = atk.pc;
                like += 999999;
            }
            now.garbage.fight_lines(attack);
            // std::string output = "";
            //	for (int i = 24; i >= 0; i--) {
            //		bool minoAreaY = i >= path.coord.y && i < path.coord.y + 4;
            //		char buffer[256] = { '\0' };
            //		sprintf(buffer, "%2d|", i);
            //		output += buffer;
            //		for (int j = 0; j < map.width; j++) {
            //			bool minoAreaX = j >= path.coord.x && j < path.coord.x + 4;
            //			/*bool minoPixel = minoAreaX && minoAreaY && test.mino_full(Minodata::MINOTYPES[block.coord.t][block.coord.r], j - block.coord.x, i - block.coord.y);*/
            //			bool minoPixel = minoAreaX && minoAreaY && (Minodata::MINOTYPES[path.coord.t][path.coord.r][3 - (i - path.coord.y)] & (8 >> (j - path.coord.x)));
            //			output += (map.full(j, i)) || minoPixel ? minoPixel ? "()" : "[]" : "  ";
            //		}
            //		output += "|\n";
            //	}
            //	printf("%s", output.c_str());
            double rating = -map.roof * p.roof;
            rating -= eval.col_trans * p.col_trans;
            rating -= eval.row_trans * p.row_trans;
            rating -= eval.aggregate_height * p.aggregate_height;
            rating -= eval.bumpiness * p.bumpiness;
            rating -= eval.hole_count * p.hole_count;
            rating -= eval.hole_line * p.hole_line;
            rating += eval.spin_slot * p.aspin_slot;
            rating += like * 10;
            rating += attack * p.attack;
            rating += now.b2b * p.b2b;
            rating += now.combo * p.combo;
            if ((last.spin_type == 3 || last.clear == 4) && (now.spin_type == 3 || now.clear == 4) && last.clear && now.clear)
            {
                rating += 999 * now.combo;
            }
            if (dead)
            {
                rating -= 9999999;
            }
            return rating;
        }
    };
    struct TetrisInstructor
    {
        const TetrisMap &map;
        TetrisMinocacheMini &move_cache;
        TetrisMino &mino;
        TetrisInstructor(const TetrisMap &map, uint8_t type) : map(map), move_cache(TetrisMinoManager::move_cache[type]), mino(TetrisMinoManager::mino_list[type]) {}
        void change_minotype(uint8_t type)
        {
            move_cache = TetrisMinoManager::move_cache[type];
            mino = TetrisMinoManager::mino_list[type];
        }
        bool integrate(const int8_t &x, const int8_t &y, const int8_t &r) const
        {
            for (int8_t i = -mino.down_offset[r], t = 4 + mino.up_offset[r]; i < t; ++i)
            {
                if (map.board[y + i] & move_cache[r][x][i])
                {
                    return false;
                }
            }
            return true;
        }
        bool l(TetrisActive &active) const
        {
            active.x -= 1;
            if (active.x < mino.left_offset[active.r])
            {
                active.x += 1;
                return false;
            }
            if (active.y > map.roof)
            {
                active.path += 'l';
                active.last_rotate = false;
                return true;
            }
            if (!integrate(active.x, active.y, active.r))
            {
                active.x += 1;
                return false;
            }
            active.last_rotate = false;
            active.path += 'l';
            return true;
        }
        bool r(TetrisActive &active) const
        {
            active.x += 1;
            if (active.x > map.width - mino.right_offset[active.r] - 4)
            {
                active.x -= 1;
                return false;
            }
            if (active.y > map.roof)
            {
                active.path += 'r';
                active.last_rotate = false;
                return true;
            }
            if (!integrate(active.x, active.y, active.r))
            {
                active.x -= 1;
                return false;
            }
            active.path += 'r';
            active.last_rotate = false;
            return true;
        }
        bool L(TetrisActive &active) const
        {
            if (active.x == mino.left_offset[active.r])
                return false;
            if (active.y > map.roof)
            {
                active.x = mino.left_offset[active.r];
                active.last_rotate = false;
                active.path += 'L';
                return true;
            }
            int count = 0;
            while (--active.x >= mino.left_offset[active.r] && integrate(active.x, active.y, active.r))
            {
                ++count;
            }
            if (count)
            {
                active.last_rotate = false;
                active.path += 'L';
            }
            ++active.x;
            return count;
        }
        bool R(TetrisActive &active) const
        {
            int8_t rightmost = map.width - mino.right_offset[active.r] - 4;
            if (active.x == rightmost)
                return false;
            if (active.y > map.roof)
            {
                active.x = rightmost;
                active.last_rotate = false;
                active.path += 'R';
                return true;
            }
            int count = 0;
            while (++active.x <= rightmost && integrate(active.x, active.y, active.r))
            {
                ++count;
            }
            if (count)
            {
                active.last_rotate = false;
                active.path += 'R';
            }
            --active.x;
            return count;
        }
        bool d(TetrisActive &active) const
        {
            if (active.y > map.roof)
            {
                active.path.append(active.y - map.roof, 'd');
                active.y = map.roof;
                active.last_rotate = false;
                return true;
            }
            active.y -= 1;
            if (active.y < mino.down_offset[active.r] || !integrate(active.x, active.y, active.r))
            {
                active.y += 1;
                return false;
            }
            active.last_rotate = false;
            active.path += 'd';
            return true;
        }
        bool D(TetrisActive &active) const
        {
            if (active.y == mino.down_offset[active.r])
                return false;
            int count = 0;
            if (active.y > map.roof)
            {
                active.y = map.roof;
                active.last_rotate = false;
                ++count;
            }
            while (--active.y >= mino.down_offset[active.r])
            {
                ++count;
                if (!integrate(active.x, active.y, active.r))
                {
                    break;
                }
            }
            if (count)
            {
                active.last_rotate = false;
                active.path += 'D';
            }
            ++active.y;
            return count;
        }
        bool D_PRESERVE_PATH(TetrisActive &active) const
        {
            if (active.y == mino.down_offset[active.r])
                return false;
            int count = 0;
            if (active.y > map.roof)
            {
                active.y = map.roof;
                active.last_rotate = false;
                ++count;
            }
            while (--active.y >= mino.down_offset[active.r])
            {
                ++count;
                if (!integrate(active.x, active.y, active.r))
                {
                    break;
                }
            }
            if (count)
            {
                active.last_rotate = false;
            }
            ++active.y;
            return count;
        }
        void D_PRESERVE_LAST_ROTATE(TetrisActive &active) const
        {
            if (active.y == mino.down_offset[active.r])
                return;
            if (active.y > map.roof)
            {
                active.y = map.roof;
            }
            while (--active.y >= mino.down_offset[active.r])
            {
                if (!integrate(active.x, active.y, active.r))
                {
                    break;
                }
            }
            ++active.y;
            return;
        }
        bool c(TetrisActive &active) const
        {
            active.r = (active.r + 1) & 3;
            int8_t rightmost = map.width - mino.right_offset[active.r] - 4;
            if (active.x >= mino.left_offset[active.r] && active.x <= rightmost && active.y >= mino.down_offset[active.r])
            {
                if (integrate(active.x, active.y, active.r))
                {
                    active.path += 'c';
                    active.last_rotate = true;
                    active.last_kick = -1;
                    return true;
                }
            }
            int8_t x = 0;
            int8_t y = 0;
            for (std::size_t i = 0; i < mino.rotate_right[active.r].size(); i++)
            {
                x = active.x + mino.rotate_right[active.r][i].first;
                y = active.y + mino.rotate_right[active.r][i].second;
                if (x < mino.left_offset[active.r] || x > rightmost || y < mino.down_offset[active.r])
                    continue;
                if (integrate(x, y, active.r))
                {
                    active.x = x;
                    active.y = y;
                    active.path += 'c';
                    active.last_rotate = true;
                    active.last_kick = i;
                    return true;
                }
            }
            active.r = (active.r - 1) & 3;
            return false;
        }
        bool z(TetrisActive &active) const
        {
            active.r = (active.r - 1) & 3;
            int8_t rightmost = map.width - mino.right_offset[active.r] - 4;
            if (active.x >= mino.left_offset[active.r] && active.x <= rightmost && active.y >= mino.down_offset[active.r])
            {
                if (integrate(active.x, active.y, active.r))
                {
                    active.path += 'z';
                    active.last_rotate = true;
                    active.last_kick = -1;
                    return true;
                }
            }
            int8_t x = 0;
            int8_t y = 0;
            for (std::size_t i = 0; i < mino.rotate_left[active.r].size(); i++)
            {
                x = active.x + mino.rotate_left[active.r][i].first;
                y = active.y + mino.rotate_left[active.r][i].second;
                if (x < mino.left_offset[active.r] || x > rightmost || y < mino.down_offset[active.r])
                    continue;
                if (integrate(x, y, active.r))
                {
                    active.x = x;
                    active.y = y;
                    active.path += 'z';
                    active.last_rotate = true;
                    active.last_kick = i;
                    return true;
                }
            }
            active.r = (active.r + 1) & 3;
            return false;
        }
        bool x(TetrisActive &active) const
        {
            active.r = (active.r + 2) & 3;
            int8_t rightmost = map.width - mino.right_offset[active.r] - 4;
            if (active.x >= mino.left_offset[active.r] && active.x <= rightmost && active.y >= mino.down_offset[active.r])
            {
                if (integrate(active.x, active.y, active.r))
                {
                    active.path += 'x';
                    active.last_rotate = true;
                    active.last_kick = -1;
                    return true;
                }
            }
            int8_t x = 0;
            int8_t y = 0;
            for (std::size_t i = 0; i < mino.rotate_180[active.r].size(); i++)
            {
                x = active.x + mino.rotate_180[active.r][i].first;
                y = active.y + mino.rotate_180[active.r][i].second;
                if (x < mino.left_offset[active.r] || x > rightmost || y < mino.down_offset[active.r])
                    continue;
                if (integrate(x, y, active.r))
                {
                    active.x = x;
                    active.y = y;
                    active.path += 'x';
                    active.last_rotate = true;
                    active.last_kick = i;
                    return true;
                }
            }
            active.r = (active.r - 2) & 3;
            return false;
        }
        bool test_up(const TetrisActive &active) const
        {
            int8_t test_y = active.y + 1;
            if (test_y > map.roof)
                return true;
            return integrate(active.x, test_y, active.r);
        }
        bool test_down(const TetrisActive &active) const
        {
            int8_t test_y = active.y - 1;
            if (test_y < mino.down_offset[active.r])
                return false;
            return integrate(active.x, test_y, active.r);
        }
        bool test_left(const TetrisActive &active) const
        {
            int8_t test_x = active.x - 1;
            if (test_x < mino.left_offset[active.r])
                return false;
            return integrate(test_x, active.y, active.r);
        }
        bool test_right(const TetrisActive &active) const
        {
            int8_t test_x = active.x + 1;
            int8_t rightmost = map.width - mino.right_offset[active.r] - 4;
            if (test_x > rightmost)
                return false;
            return integrate(test_x, active.y, active.r);
        }
        bool immobile(TetrisActive &active) const
        {
            return !test_down(active) && !test_left(active) && !test_right(active) && !test_up(active);
        }
        void build_snapshot(TetrisActive &active) const
        {
            TetrisActive copy = active;
            D_PRESERVE_LAST_ROTATE(copy);
            memcpy(active.snapshot, map.board, sizeof(map.board));
            for (int i = copy.y; i < copy.y + 4; i++)
            {
                if (i < 0)
                    continue;
                active.snapshot[i] |= move_cache[copy.r][copy.x][i - copy.y];
            }
        }
        void attach(TetrisMap &map_copy, TetrisActive &active) const
        {
            memcpy(map_copy.board, active.snapshot, sizeof(map_copy.board));
        }
        bool check_death(TetrisActive &active) const
        {
            return !integrate(active.x, active.y, active.r);
        }
    };
    struct TetrisNode
    {
        double rating;
        std::size_t version;
        bool running;
        bool complete;
        TetrisNode *parent;
        TetrisMap map;
        TetrisStatus status;
        std::vector<TetrisNode *> children;
        TetrisNode(std::size_t version, TetrisNode *parent, TetrisMap &map, TetrisStatus &status) : version(version), parent(parent), map(map), status(status)
        {
            running = false;
            complete = false;
            rating = 0;
        }
        TetrisNode(double rating, std::size_t version, TetrisNode *parent, TetrisMap &map, TetrisStatus &status) : rating(rating), version(version), parent(parent), map(map), status(status)
        {
            running = false;
            complete = false;
        }
        double calc_rating(std::queue<TetrisActive> &path)
        {
            if (parent != nullptr)
            {
                double acc_rating = parent->calc_rating(path);
                path.push(status.next.active);
                return acc_rating + rating;
            }
            return rating;
        }
    };
    struct TetrisNodeSorter
    {
        bool operator()(const TetrisNode *a, const TetrisNode *b)
        {
            return a->rating > b->rating;
        }
    };
    struct TetrisPathManager
    {
        std::queue<TetrisActive> search;
        std::unordered_set<TetrisActive, TetrisCoordHash> visited;
        std::vector<TetrisActive> result;
        std::vector<std::function<bool(TetrisActive &)>> moves;
        TetrisInstructor instructor;
        TetrisConfig &config;
        TetrisPathManager(TetrisActive active, TetrisConfig &config, TetrisMap &map) : instructor(map, active.type), config(config)
        {
            search.push(active);
            visited.insert(active);

            if (config.allow_lr)
            {
                moves.push_back([this](TetrisActive &a)
                                { return instructor.l(a); });
                moves.push_back([this](TetrisActive &a)
                                { return instructor.r(a); });
            }
            if (config.allow_LR)
            {
                moves.push_back([this](TetrisActive &a)
                                { return instructor.L(a); });
                moves.push_back([this](TetrisActive &a)
                                { return instructor.R(a); });
            }
            if (config.allow_d)
                moves.push_back([this](TetrisActive &a)
                                { return instructor.d(a); });
            if (config.allow_D)
                moves.push_back([this](TetrisActive &a)
                                { return instructor.D(a); });
            if (config.allow_180)
                moves.push_back([this](TetrisActive &a)
                                { return instructor.x(a); });
            moves.push_back([this](TetrisActive &a)
                            { return instructor.c(a); });
            moves.push_back([this](TetrisActive &a)
                            { return instructor.z(a); });
        }
        void init_new(TetrisActive &active, TetrisMinocacheMini &move_cache, TetrisMino &mino)
        {
            visited.clear();
            result.clear();
            search.push(active);
            visited.insert(active);
            instructor.move_cache = move_cache;
            instructor.mino = mino;
        }
        void test_run()
        {
            TetrisActive current, temp;
            while (!search.empty())
            {
                current = search.front();
                search.pop();

                for (auto &move : moves)
                {
                    temp = current;
                    if (move(temp) && visited.find(temp) == visited.end())
                    {
                        search.push(temp);
                        visited.insert(temp);
                    }
                }

                instructor.build_snapshot(current);
                if (std::find(result.rbegin(), result.rend(), current.snapshot) == result.rend())
                {
                    result.push_back(current);
                }
            }
        }
        TetrisActive run_lite(TetrisMap map, TetrisStatus status, uint8_t mino, TetrisConfig config)
        {
            double best = std::numeric_limits<double>::lowest();
            TetrisActive best_active;
            TetrisActive current(config.default_x, config.default_y, 0, mino), temp;
            std::vector<std::function<bool(TetrisActive &)>> moves;

            if (config.allow_lr)
            {
                moves.push_back([this](TetrisActive &a)
                                { return instructor.l(a); });
                moves.push_back([this](TetrisActive &a)
                                { return instructor.r(a); });
            }
            if (config.allow_LR)
            {
                moves.push_back([this](TetrisActive &a)
                                { return instructor.L(a); });
                moves.push_back([this](TetrisActive &a)
                                { return instructor.R(a); });
            }
            if (config.allow_d)
                moves.push_back([this](TetrisActive &a)
                                { return instructor.d(a); });
            if (config.allow_D)
                moves.push_back([this](TetrisActive &a)
                                { return instructor.D(a); });
            if (config.allow_180)
                moves.push_back([this](TetrisActive &a)
                                { return instructor.x(a); });
            moves.push_back([this](TetrisActive &a)
                            { return instructor.c(a); });
            moves.push_back([this](TetrisActive &a)
                            { return instructor.z(a); });
            while (!search.empty())
            {
                current = search.front();
                search.pop();

                for (auto &move : moves)
                {
                    temp = current;
                    if (move(temp) && visited.find(temp) == visited.end())
                    {
                        search.push(temp);
                        visited.insert(temp);
                    }
                }

                instructor.build_snapshot(current);
                if (std::find(result.rbegin(), result.rend(), current.snapshot) == result.rend())
                {
                    result.push_back(current);
                    TetrisMap map_copy = map;
                    TetrisStatus status_copy = status;
                    status_copy.next.active = current;
                    status_copy.spin_type = instructor.immobile(current) ? 3 : 0;
                    instructor.attach(map_copy, current);
                    status_copy.clear = map_copy.flush();
                    map_copy.scan();
                    TetrisActive next = TetrisActive(config.default_x, config.default_y, config.default_r, status.next.queue.front());
                    bool dead = instructor.check_death(next);
                    double rating = TetrisJudge::begin_judgement(status, status_copy, map_copy, dead);
                    if (rating > best)
                    {
                        best = rating;
                        best_active = current;
                    }
                }
            }
            return best_active;
        }
        void run(TetrisNode *node, TetrisNextManager &next_manager)
        {
            TetrisActive current, temp;
            while (!search.empty())
            {
                current = search.front();
                search.pop();

                for (auto &move : moves)
                {
                    temp = current;
                    if (move(temp) && visited.find(temp) == visited.end())
                    {
                        search.push(temp);
                        visited.insert(temp);
                    }
                }

                instructor.build_snapshot(current);
                if (std::find(result.rbegin(), result.rend(), current.snapshot) == result.rend())
                {
                    result.push_back(current);
                    TetrisMap map_copy = node->map;
                    TetrisStatus status_copy = node->status;
                    status_copy.next = next_manager;
                    status_copy.next.active = current;
                    status_copy.spin_type = instructor.immobile(current) ? 3 : 0;
                    instructor.attach(map_copy, current);
                    status_copy.clear = map_copy.flush();
                    map_copy.scan();
                    TetrisActive next = TetrisActive(config.default_x, config.default_y, config.default_r, next_manager.queue.front());
                    bool dead = instructor.check_death(next);
                    double rating = TetrisJudge::begin_judgement(node->status, status_copy, map_copy, dead);
                    node->children.push_back(new TetrisNode(rating, node->version + 1, node, map_copy, status_copy));
                }
            }
        }
    };
    using TetrisNodeResult = std::pair<double, std::queue<TetrisActive>>;
    struct TetrisTree
    {
        TetrisNode *root;
        std::queue<TetrisNode *> tasks;
        TetrisConfig &config;
        std::size_t stable_version;
        TetrisNodeResult stable, beta;
        TetrisMinoManager mino_manager;
        TetrisNextManager next_manager;
        TetrisTree(TetrisMap &map, TetrisConfig &config, TetrisStatus status) : config(config), mino_manager("botris_srs.json"), next_manager(config)
        {
            map.scan();
            stable_version = 0;
            root = new TetrisNode(stable_version, nullptr, map, status);
            tasks.push(root);
            stable.first = std::numeric_limits<double>::lowest();
            beta.first = std::numeric_limits<double>::lowest();
        }
        void kill_node(TetrisNode *node)
        {
            for (auto &child : node->children)
            {
                kill_node(child);
            }
            delete node;
        }
        void update_task(TetrisNode *node)
        {
            if (node->children.empty())
            {
                return;
            }
            std::sort(node->children.begin(), node->children.end(), TetrisNodeSorter());
            while (node->children.size() > config.beam_width)
            {
                kill_node(node->children.back());
                node->children.pop_back();
            }
            for (auto child : node->children)
            {
                tasks.push(child);
            }
        }
        void compare_and_update(TetrisNode *node)
        {
            if (node->version >= stable_version)
            {
                ++stable_version;
                stable = beta;
                beta.first = std::numeric_limits<double>::lowest();
            }
            TetrisNodeResult alpha;
            alpha.first = node->calc_rating(alpha.second);
            if (!node->children.empty())
            {
                alpha.first += node->children[0]->rating;
                alpha.second.push(node->children[0]->status.next.active);
            }
            if (alpha.first > beta.first)
            {
                beta = alpha;
            }
            node->complete = true;
            node->running = false;
        }
        void expand(TetrisNode *node)
        {
            if (node->running || node->complete)
            {
                return;
            }
            node->running = true;
            next_manager = node->status.next;
            if (node->parent != nullptr)
            {
                if (!next_manager.next())
                {
                    if (!next_manager.change_hold())
                    {
                        compare_and_update(node);
                        return;
                    }
                }
            }
            {
                TetrisPathManager path(next_manager.active, config, node->map);
                path.run(node, next_manager);
            }
            if (config.can_hold && !node->status.next.changed_hold)
            {
                if (next_manager.change_hold())
                {
                    TetrisPathManager path(next_manager.active, config, node->map);
                    path.run(node, next_manager);
                }
            }
            update_task(node);
            compare_and_update(node);
        }
        std::queue<TetrisActive> run()
        {
            auto now = std::chrono::high_resolution_clock::now();
            while ((stable.first == std::numeric_limits<double>::lowest() ||
                    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now).count() < config.target_time) &&
                   !tasks.empty())
            {
                expand(tasks.front());
                tasks.pop();
            }
            kill_node(root);
            return stable.second;
        }
    };
}