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
#if !_MSC_VER
#define _mm_popcnt_u32 __builtin_popcount
#endif
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
    std::unordered_map<char, uint8_t> char_to_type = {
        {'S', S},
        {'L', L},
        {'Z', Z},
        {'I', I},
        {'T', T},
        {'O', O},
        {'J', J},
        {' ', EMPTY}};
    std::unordered_map<uint8_t, char> type_to_char = {
        {S, 'S'},
        {L, 'L'},
        {Z, 'Z'},
        {I, 'I'},
        {T, 'T'},
        {O, 'O'},
        {J, 'J'},
        {EMPTY, ' '}};
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
        TetrisMap() {}
        TetrisMap(const uint8_t width, const uint8_t height) : height(height), width(width), roof(0)
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
            const uint32_t line = CLEAR_REQUIREMENT[width] ^ (1 << index);
            for (uint8_t i = 0; i < amount; i++)
            {
                board[i] = line;
            }
        }
        constexpr bool full(const uint8_t &x, const uint8_t &y) const
        {
            return board[y] & BINARY_TEMPLATE[x];
        }
        void scan()
        {
            roof = height - 1;
            while (board[roof - 1] == 0 && roof > 0)
            {
                roof--;
            };
        }
        uint8_t flush()
        {
            uint8_t clear = 0;
            const auto &req = CLEAR_REQUIREMENT[width - 1];
            const auto &height_m1 = height - 1;
            for (int8_t i = height_m1; i >= 0; i--)
            {
                if ((board[i] & req) == req)
                {
                    std::memmove(&board[i], &board[i + 1], sizeof(uint32_t) * (height - i));
                    board[height_m1] = 0;
                    clear++;
                }
            }
            scan();
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
        bool allow_x;
        time_t target_time;
        TetrisConfig() : default_x(3), default_y(17), default_r(0), can_hold(true), allow_LR(true), allow_lr(true), allow_D(true), allow_d(true), allow_x(false), target_time(100) {}
    };
    struct TetrisCoord
    {
        int8_t x;
        int8_t y;
        int8_t r;
        TetrisCoord() {}
        TetrisCoord(const int8_t &x, const int8_t &y, const int8_t &r) : x(x), y(y), r(r) {}
        TetrisCoord(const TetrisConfig &config) : x(config.default_x), y(config.default_y), r(config.default_r) {}
    };

    struct TetrisActive : public TetrisCoord
    {
        uint8_t type;
        bool last_rotate;
        int8_t last_kick;
        std::string path;
        uint32_t snapshot;
        TetrisActive() {}
        TetrisActive(const int8_t &x, const int8_t &y, const int8_t &r, const uint8_t &type) : TetrisCoord(x, y, r), type(type), last_rotate(false), last_kick(-1) {}
    };
    struct TetrisNext
    {
        TetrisActive active;
        std::queue<uint8_t> queue;
        uint8_t hold;
        bool changed_hold;
        std::size_t size() const
        {
            return queue.size();
        }
        void push(uint8_t &type)
        {
            queue.push(type);
        }
    };
    struct TetrisNextManager : public TetrisNext
    {
        TetrisConfig config;
        TetrisNextManager(TetrisConfig &config) : TetrisNext(), config(config) {}
        void operator=(TetrisNext &next)
        {
            active = next.active;
            queue = next.queue;
            hold = next.hold;
            changed_hold = next.changed_hold;
        }
        bool change_hold()
        {
            if (!config.can_hold || changed_hold)
                return false;

            if (hold != EMPTY)
            {
                if (hold == active.type)
                    return false;
                uint8_t temp = hold;
                hold = active.type;
                active = TetrisActive(config.default_x, config.default_y, config.default_r, temp);
            }
            else
            {
                if (queue.empty())
                    return false;
                hold = active.type;
                active = TetrisActive(config.default_x, config.default_y, config.default_r, queue.front());
                queue.pop();
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
        bool empty()
        {
            return queue.empty();
        }
        std::size_t size()
        {
            return queue.size();
        }
        std::string to_string()
        {
            std::string result;
            std::queue<uint8_t> temp = queue;
            while (!temp.empty())
            {
                result += type_to_char[temp.front()];
                temp.pop();
            }
            return result;
        }
    };
    struct TetrisPendingLine
    {
        uint8_t lines;
        uint8_t at_depth;
        TetrisPendingLine(uint8_t lines, uint8_t at_depth) : lines(lines), at_depth(at_depth) {}
    };
    struct TetrisPendingLineManager
    {
        std::mt19937 gen;
        std::deque<TetrisPendingLine> pending;
        void fight_lines(uint8_t &attack)
        {
            while (!pending.empty() && attack > 0)
            {
                if (pending.front().lines > attack)
                {
                    pending.front().lines -= attack;
                    attack = 0;
                }
                else
                {
                    attack -= pending.front().lines;
                    pending.pop_front();
                }
            }
        }
        void decay()
        {
            for (auto &line : pending)
            {
                line.at_depth--;
            }
        }
        uint16_t estimate_damage() const
        {
            uint16_t damage = 0;
            for (auto &line : pending)
            {
                if (line.at_depth <= 0)
                {
                    damage += line.lines;
                }
                else
                {
                    break;
                }
            }
            return damage;
        }
        uint16_t total_damage() const
        {
            return std::accumulate(pending.begin(), pending.end(), 0, [](uint8_t sum, const TetrisPendingLine &line)
                                   { return sum + line.lines; });
        }
        void take_all_damage(TetrisMap &map, const double &messiness)
        {
            static std::uniform_int_distribution<int> dis(0, map.width - 1);
            static std::uniform_int_distribution<int> mess_dis(0, 99);
            while (!pending.empty() && pending[0].at_depth <= 0)
            {
                uint8_t line = pending[0].lines;
                uint8_t index = dis(gen);
                pending.pop_front();
                for (int8_t i = 0; i < line; i++)
                {
                    if (mess_dis(gen) < messiness * 100)
                    {
                        index = dis(gen);
                    }
                    map.push_cheese(1, index);
                }
            }
            map.scan();
        }
        void push_lines(uint8_t line, uint8_t at_depth)
        {
            pending.emplace_back(TetrisPendingLine(line, at_depth));
        }
        TetrisPendingLineManager() {}
        TetrisPendingLineManager(std::mt19937 &gen) : gen(gen) {}
        TetrisPendingLineManager(int16_t seed) : gen(seed) {}
    };
    struct TetrisStatus
    {
        uint16_t b2b;
        uint16_t combo;
        uint8_t clear;
        uint8_t attack;
        uint8_t send_attack;
        uint32_t allspin_value;
        bool allspin;
        bool dead;
        TetrisNext next;
        TetrisPendingLineManager garbage;
        TetrisStatus() {}
        TetrisStatus(uint16_t b2b, uint16_t combo, TetrisNext &next, TetrisPendingLineManager &garbage) : b2b(b2b), combo(combo), clear(0), attack(0), send_attack(0), allspin_value(0), allspin(false), dead(false), next(next), garbage(garbage) {}
        void init()
        {
            b2b = 0;
            combo = 0;
            clear = 0;
            attack = 0;
            send_attack = 0;
            allspin_value = 0;
            allspin = false;
            dead = false;
            next = TetrisNext();
            garbage = TetrisPendingLineManager(rand() % INT32_MAX);
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
        uint8_t combo_table[32] = {0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
    } atk;
    enum TetrisWeightEnum
    {
        ALLSPIN_1,
        ALLSPIN_2,
        ALLSPIN_3,
        ALLSPIN_SLOT,
        COMBO,
        ATTACK,
        CLEAR_1,
        CLEAR_2,
        CLEAR_3,
        CLEAR_4,
        B2B,
        ROOF,
        COL_TRANS,
        ROW_TRANS,
        HOLE_COUNT,
        HOLE_LINE,
        WIDE_2,
        WIDE_3,
        WIDE_4,
        HIGH_WIDING,
        AGGREGATE_HEIGHT,
        BUMPINESS,
        HOLD_I,
        HOLD_SZO,
        HOLD_LJT,
        WASTE_I,
        WASTE_SZO,
        WASTE_LJT,
        TANK,
        MID_GROUND,
        HIGH_GROUND,
        SEND,
        CANCEL,
        SKIM,
        APL,
        END_OF_PARAM
    };
    struct TetrisParam
    {
        double weight[64];
        TetrisParam()
        {
            memset(weight, 0, sizeof(weight));
            weight[ALLSPIN_1] = 60;
            weight[ALLSPIN_2] = 48;
            weight[ALLSPIN_3] = 32;
            weight[ALLSPIN_SLOT] = 35;
            weight[COMBO] = 60;
            weight[ATTACK] = 60;
            weight[CLEAR_1] = 0;
            weight[CLEAR_2] = 0;
            weight[CLEAR_3] = 0;
            weight[CLEAR_4] = 32;
            weight[B2B] = 12;
            weight[ROOF] = 0;
            weight[COL_TRANS] = -1.1;
            weight[ROW_TRANS] = 60;
            weight[HOLE_COUNT] = 60;
            weight[HOLE_LINE] = 30;
            weight[WIDE_2] = 4;
            weight[WIDE_3] = 8;
            weight[WIDE_4] = 12;
            weight[HIGH_WIDING] = 1;
            weight[AGGREGATE_HEIGHT] = 2.5;
            weight[BUMPINESS] = 1;
            weight[HOLD_I] = 0.8;
            weight[HOLD_SZO] = 0.4;
            weight[HOLD_LJT] = 0.2;
            weight[WASTE_I] = 0.2;
            weight[WASTE_SZO] = 0.4;
            weight[WASTE_LJT] = 0.8;
            weight[TANK] = 40;
            weight[MID_GROUND] = 0.7;
            weight[HIGH_GROUND] = 0.3;
            weight[SEND] = 20;
            weight[CANCEL] = 40;
            weight[SKIM] = 10;
            weight[APL] = 40;
            // the rest depends on pso
        }
        bool operator==(const TetrisParam &other) const
        {
            return std::memcmp(weight, other.weight, sizeof(weight)) == 0;
        }
        bool operator!=(const TetrisParam &other) const
        {
            return std::memcmp(weight, other.weight, sizeof(weight)) != 0;
        }
    };
    struct TetrisInstructor
    {
        TetrisMap &map;
        TetrisMinocacheMini &move_cache;
        TetrisMino &mino;
        TetrisInstructor(TetrisMap &map, uint8_t &type) : map(map), move_cache(TetrisMinoManager::move_cache[type]), mino(TetrisMinoManager::mino_list[type]) {}
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
        bool consumer_d(TetrisActive &active) const
        {
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
            while (--active.y >= mino.down_offset[active.r] && integrate(active.x, active.y, active.r))
            {
                ++count;
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
            while (--active.y >= mino.down_offset[active.r] && integrate(active.x, active.y, active.r))
            {
                ++count;
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
            while (--active.y >= mino.down_offset[active.r] && integrate(active.x, active.y, active.r))
                ;
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
            for (std::size_t i = 0, max = mino.rotate_right[active.r].size(); i < max; i++)
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
            for (std::size_t i = 0, max = mino.rotate_left[active.r].size(); i < max; i++)
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
            for (std::size_t i = 0, max = mino.rotate_180[active.r].size(); i < max; i++)
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
        bool immobile(const TetrisActive &active) const
        {
            return active.last_rotate && !test_down(active) && !test_left(active) && !test_right(active) && !test_up(active);
        }
        void build_snaphash(TetrisActive &active) const
        {
            TetrisActive copy = active;
            D_PRESERVE_LAST_ROTATE(copy);
            copy.x -= mino.left_offset[copy.r];
            copy.y -= mino.down_offset[copy.r];
            active.snapshot = 5381;
            for (int8_t i = -mino.down_offset[copy.r], j = 4 + mino.up_offset[copy.r]; i < j; i++)
            {
                active.snapshot = ((active.snapshot << 5) + active.snapshot) + _mm_popcnt_u32(move_cache[active.r][active.x][i]);
            }
            active.snapshot += copy.x * (31 + copy.y);
        }
        void attach(TetrisMap &map_copy, const TetrisActive &active) const
        {
            TetrisActive copy = active;
            D_PRESERVE_LAST_ROTATE(copy);
            for (int i = std::max<int>(0, copy.y); i < copy.y + 4; i++)
            {
                map_copy.board[i] |= move_cache[copy.r][copy.x][i - copy.y];
            }
        }
        void dropless_attach(TetrisMap &map_copy, const TetrisActive &active) const
        {
            for (int i = std::max<int>(0, active.y); i < active.y + 4; i++)
            {
                map_copy.board[i] |= move_cache[active.r][active.x][i - active.y];
            }
        }
        bool check_death(const TetrisMap &map_ext, const TetrisActive &active) const
        {
            TetrisMinocacheMini &cache = TetrisMinoManager::move_cache[active.type];
            TetrisMino &new_mino = TetrisMinoManager::mino_list[active.type];
            for (int8_t i = -new_mino.down_offset[active.r], t = 4 + new_mino.up_offset[active.r]; i < t; ++i)
            {
                if (map_ext.board[active.y + i] & cache[active.r][active.x][i])
                {
                    return true;
                }
            }
            return false;
        }
    };
    struct TetrisEvaluation
    {
        TetrisParam &p;
        TetrisEvaluation(TetrisParam &p) : p(p) {}
        static void find_every_spin(const TetrisMap &map, uint32_t &val)
        {
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
                    int xm1 = x - 1;
                    int x1 = x + 1;
                    int x2 = x + 2;
                    int x3 = x + 3;
                    int x4 = x + 4;
                    if (x < map.width - 2)
                    {
                        // S spin
                        if (~row0 & ref[x] && ~row0 & ref[x1] && ~row1 & ref[x1] && ~row1 & ref[x2])
                        {
                            // double/single
                            if ((row0 & ref[x2] || (y == 0 || rowm1 & ref[x1])) && (row1 & ref[x] || (row2 & ref[x2] && (x == 0 || row0 & ref[xm1]))))
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
                        // S spin
                        if (~row2 & ref[x] && ~row1 & ref[x] && ~row1 & ref[x1] && ~row0 & ref[x1])
                        {
                            // triple
                            if (row0 & ref[x] && (row2 & ref[x1] || (row0 & ref[x2] && row3 & ref[x])))
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
                        // Z spin
                        if (~row1 & ref[x] && ~row1 & ref[x1] && ~row0 & ref[x1] && ~row0 & ref[x2])
                        {
                            // double/single
                            if ((row0 & ref[x] || (y == 0 || rowm1 & ref[x])) && (row1 & ref[x2] || (row2 & ref[x] && (x3 == map.width || row0 & ref[x3]))))
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
                        if (~row2 & ref[x1] && ~row1 & ref[x1] && ~row1 & ref[x] && ~row0 & ref[x])
                        {
                            // triple
                            if (row0 & ref[x1] && (row2 & ref[x] || ((x != 0 && row0 & ref[xm1]) && row3 & ref[x1])))
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
                        // L spin
                        if (~row0 & ref[x] && ~row0 & ref[x1] && ~row0 & ref[x2] && ~row1 & ref[x2])
                        {
                            // double
                            bool cond1 = row1 & ref[x1] && (x3 == map.width || row0 & ref[x3] || row1 & ref[x3]);
                            bool cond2 = (x3 == map.width || row1 & ref[x3]) && row1 & ref[x];
                            bool cond3 = y == 0 || rowm1 & ref[x] || rowm1 & ref[x1] || rowm1 & ref[x2];
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
                        // L spin
                        if (~row0 & ref[x] && ~row1 & ref[x] && ~row1 & ref[x1] && ~row1 & ref[x2])
                        {
                            // double, 180 facing
                            bool cond1 = (x == 0 || row1 & ref[xm1]) && row0 & ref[x1] && row2 & ref[x2];
                            bool cond2 = x != 0 && row0 & ref[xm1] && row0 & ref[x1] && row2 & ref[x1];
                            bool cond3 = (x == 0 || row1 & ref[xm1] || row0 & ref[xm1]) && row0 & ref[x1] && row2 & ref[x1];
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
                        // L spin
                        if (~row0 & ref[x] && ~row1 & ref[x] && ~row2 & ref[x] && ~row0 & ref[x1])
                        {
                            // triple
                            bool cond1 = x == 0 || row0 & ref[xm1] || row1 & ref[xm1] || row2 & ref[xm1];
                            bool cond2 = row1 & ref[x1] || (row3 & ref[x] && row0 * ref[x2]);
                            bool cond3 = y == 0 || rowm1 & ref[x] || rowm1 & ref[x1];
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
                        // J spin
                        if (~row0 & ref[x] && ~row0 & ref[x1] && ~row0 & ref[x2] && ~row1 & ref[x])
                        {
                            // double
                            bool cond1 = row1 & ref[x1] && (x == 0 || row0 & ref[xm1] || row1 & ref[xm1]);
                            bool cond2 = (x == 0 || row1 & ref[xm1]) && row1 & ref[x2];
                            bool cond3 = y == 0 || rowm1 & ref[x] || rowm1 & ref[x1] || rowm1 & ref[x2];
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
                        // J spin
                        if (~row1 & ref[x] && ~row1 & ref[x1] && ~row1 & ref[x2] && ~row0 & ref[x2])
                        {
                            // double, 180 facing
                            bool cond1 = (x3 == map.width || row1 & ref[x3]) && row0 & ref[x1] && row2 & ref[x];
                            bool cond2 = x3 != map.width && row0 & ref[x3] && row0 & ref[x1] && row2 & ref[x1];
                            bool cond3 = (x3 == map.width || row1 & ref[x3] || row0 & ref[x3]) && row0 & ref[x1] && row2 & ref[x1];
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
                        // J spin
                        if (~row0 & ref[x] && ~row0 & ref[x1] && ~row1 & ref[x1] && ~row2 & ref[x1])
                        {
                            // triple
                            bool cond1 = row0 & ref[x2] || row1 & ref[x2] || row2 & ref[x2];
                            bool cond2 = row1 & ref[x];
                            bool cond3 = y == 0 || rowm1 & ref[x] || rowm1 & ref[x1];
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
                        // T spin
                        if (~row0 & ref[x1] && ~row1 & ref[x] && ~row1 & ref[x1] && ~row1 & ref[x2] && ~row2 & ref[x1])
                        {
                            // double
                            if (row0 & ref[x] && row0 & ref[x2] && (row2 & ref[x] || row2 & ref[x2]))
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
                        // T spin
                        if (~row0 & ref[x1] && ~row1 & ref[x1] && ~row2 & ref[x1])
                        {
                            if ((~row1 & ref[x] && row1 & ref[x2]) || (row1 & ref[x] && ~row1 & ref[x2]))
                            {
                                // triple
                                if (row0 & ref[x] && row0 & ref[x2] && row2 & ref[x] && row2 & ref[x2])
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
                        // T spin
                        if (~row0 & ref[x] && ~row0 & ref[x1] && ~row0 & ref[x2] && ~row1 & ref[x1])
                        {
                            // single (mini)
                            bool cond1 = row1 & ref[x] && (x3 == map.width || row0 & ref[x3]);
                            bool cond2 = row1 & ref[x2] && (x == 0 || row0 & ref[xm1]);
                            bool cond3 = y == 0 || rowm1 & ref[x] || rowm1 & ref[x1] || rowm1 & ref[x2];
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
                    // I spin
                    if (x < map.width - 3 && ~row0 & ref[x] && ~row0 & ref[x1] && ~row0 & ref[x2] && ~row0 & ref[x3])
                    {
                        // single
                        bool up_cover = row1 & ref[x] || row1 & ref[x1] || row1 & ref[x2] || row1 & ref[x3];
                        if ((x == 0 || row0 & ref[xm1]) && (x4 == map.width || row0 & ref[x4]) && up_cover)
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
                        bool left_cover = x == 0 || row0 & ref[xm1] || row1 & ref[xm1] || row2 & ref[xm1] || row3 & ref[xm1];
                        bool right_cover = x1 == map.width || row0 & ref[x1] || row1 & ref[x1] || row2 & ref[x1] || row3 & ref[x1];
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
                    // O spin
                    if (~row0 & ref[x] && ~row0 & ref[x1] && ~row1 & ref[x] && ~row1 & ref[x1])
                    {
                        bool down_cover = y == 0 || rowm1 & ref[x] || rowm1 & ref[x1];
                        bool left_cover = x == 0 || row0 & ref[xm1] || row1 & ref[xm1];
                        bool right_cover = x1 == map.width || row0 & ref[x2] || row1 & ref[x2];
                        bool up_cover = row2 & ref[x] || row2 & ref[x1];
                        if (down_cover && left_cover && right_cover && up_cover)
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
                }
            }
        }
        double begin_judgement(const TetrisStatus &last, TetrisStatus &now, TetrisMap &map, const int8_t &depth, TetrisInstructor &ins, const TetrisConfig &config)
        {
            double like = 0;
            now.attack = 0;
            switch (now.clear)
            {
            case 0:
                now.garbage.take_all_damage(map, atk.messiness);
                now.combo = 0;
                now.garbage.decay();
                switch (now.next.active.type)
                {
                case S:
                case Z:
                case O:
                    like -= p.weight[WASTE_SZO];
                    break;
                case L:
                case J:
                case T:
                    like -= p.weight[WASTE_LJT];
                case I:
                    like -= p.weight[WASTE_I];
                    break;
                }
                break;
            case 1:
                if (now.allspin)
                {
                    now.attack += atk.ass + now.b2b;
                    like += p.weight[ALLSPIN_1];
                    now.b2b = atk.b2b;
                }
                else
                {
                    like += p.weight[CLEAR_1];
                    now.attack += atk.single;
                    now.b2b = 0;
                    switch (now.next.active.type)
                    {
                    case S:
                    case Z:
                    case O:
                        like -= p.weight[WASTE_SZO];
                        break;
                    case L:
                    case J:
                    case T:
                        like -= p.weight[WASTE_LJT];
                    case I:
                        like -= p.weight[WASTE_I];
                        break;
                    }
                }
                now.attack += atk.combo_table[++now.combo];
                break;
            case 2:
                if (now.allspin)
                {
                    now.attack += atk.asd + now.b2b;
                    like += p.weight[ALLSPIN_2];
                    now.b2b = atk.b2b;
                }
                else
                {
                    like += p.weight[CLEAR_2];
                    now.attack += atk.double_;
                    now.b2b = 0;
                    switch (now.next.active.type)
                    {
                    case L:
                    case J:
                    case T:
                        like -= p.weight[WASTE_LJT];
                    case I:
                        like -= p.weight[WASTE_I];
                        break;
                    }
                }
                now.attack += atk.combo_table[++now.combo];
                break;
            case 3:
                if (now.allspin)
                {
                    now.attack += atk.ast + now.b2b;
                    like += p.weight[ALLSPIN_3];
                    now.b2b = atk.b2b;
                }
                else
                {
                    like += p.weight[CLEAR_3];
                    now.attack += atk.triple;
                    now.b2b = 0;
                    switch (now.next.active.type)
                    {
                    case I:
                        like -= p.weight[WASTE_I];
                        break;
                    }
                }
                now.attack += atk.combo_table[++now.combo];
                break;
            case 4:
                like += p.weight[CLEAR_4];
                now.attack += atk.quad + now.b2b + atk.combo_table[++now.combo];
                now.b2b = atk.b2b;
            }
            if (!now.next.queue.empty())
            {
                TetrisActive next_active(config.default_x, config.default_y, config.default_r, now.next.queue.front());
                if (ins.check_death(map, next_active))
                {
                    now.dead = true;
                }
            }
            if (now.clear != 0)
            {
                like -= p.weight[TANK] * now.garbage.estimate_damage() * depth;
            }
            struct TetrisEvalTemplate
            {
                int32_t col_trans;
                int32_t row_trans;
                int32_t aggregate_height;
                int32_t aggregate_height_arr[32];
                int16_t bumpiness;
                int16_t hole_count;
                int8_t hole_line;
                int8_t wide[32];
                bool switch_box[7];
            } eval;
            memset(&eval, 0, sizeof(eval));
            for (int i = map.roof - 1; i >= 0; --i)
            {
                uint8_t wide_max = 0;
                uint8_t wide_count = 0;
                for (int j = 0; j < map.width; j++)
                {
                    if (map.full(j, i))
                    {
                        if (eval.aggregate_height_arr[j] == 0)
                        {
                            eval.aggregate_height_arr[j] = i + 1;
                        }
                        if (wide_count > wide_max)
                        {
                            wide_max = wide_count;
                        }
                        wide_count = 0;
                    }
                    else
                    {
                        wide_count++;
                    }
                    if (j + 1 != map.width)
                    {
                        eval.row_trans += map.full(j, i) != map.full(j + 1, i);
                    }
                }
                if (_mm_popcnt_u32(map.board[i]) == map.width - wide_max)
                {
                    eval.wide[wide_max] += 2;
                }
                ++eval.wide[wide_max];
                if (i - 1 >= 0)
                {
                    int check = eval.hole_count;
                    eval.hole_count += _mm_popcnt_u32(map.board[i] & ~map.board[i - 1]);
                    eval.hole_line += check != eval.hole_count;
                    eval.col_trans += _mm_popcnt_u32(map.board[i] ^ map.board[i - 1]);
                }
            }
            now.allspin_value = 0;
            find_every_spin(map, now.allspin_value);
            for (int i = 0; i < map.width; i++)
            {
                eval.aggregate_height += eval.aggregate_height_arr[i];
                if (i != 0)
                {
                    eval.bumpiness += std::abs(eval.aggregate_height_arr[i - 1] - eval.aggregate_height_arr[i]);
                }
            }
            if (!map.roof)
            {
                now.attack = atk.pc;
                like += 999999;
            }
            now.send_attack = now.attack;
            now.garbage.fight_lines(now.send_attack);
            if (!now.allspin && last.allspin_value > now.allspin_value)
            {
                like += ((double)now.allspin_value - last.allspin_value) * p.weight[SKIM];
            }
            if (now.clear && last.garbage.total_damage() != now.garbage.total_damage())
            {
                like += ((double)last.garbage.total_damage() - now.garbage.total_damage()) * p.weight[CANCEL];
            }
            double rating = -map.roof * p.weight[ROOF];
            for (int i = map.width - 1; i >= 0; --i)
            {
                rating += eval.wide[i] * (10 - i) * p.weight[HIGH_WIDING];
            }
            switch (now.next.hold)
            {
            case I:
                like += p.weight[HOLD_I];
                break;
            case S:
            case Z:
            case O:
                like += p.weight[HOLD_SZO];
                break;
            case L:
            case J:
            case T:
                like += p.weight[HOLD_LJT];
            }
            rating -= eval.col_trans * p.weight[COL_TRANS];
            rating -= eval.row_trans * p.weight[ROW_TRANS];
            rating -= eval.aggregate_height * p.weight[AGGREGATE_HEIGHT];
            rating -= eval.bumpiness * p.weight[BUMPINESS];
            rating -= eval.hole_count * p.weight[HOLE_COUNT];
            rating -= eval.hole_line * p.weight[HOLE_LINE];
            rating += eval.wide[2] * p.weight[WIDE_2];
            rating += eval.wide[3] * p.weight[WIDE_3];
            rating += eval.wide[4] * p.weight[WIDE_4];
            double status_rating = 0;
            status_rating += now.allspin_value * p.weight[ALLSPIN_SLOT];
            status_rating += like * 10;
            status_rating += now.attack * p.weight[ATTACK];
            status_rating += now.send_attack * p.weight[SEND];
            status_rating += (now.b2b - last.b2b) * p.weight[B2B];
            status_rating += (now.attack / std::max<int>(1, now.clear)) * p.weight[APL];
            status_rating += now.attack * now.combo * p.weight[COMBO];
            status_rating *= map.roof > 7 ? map.roof > 14 ? p.weight[HIGH_GROUND] : p.weight[MID_GROUND] : 1;
            if (now.dead)
            {
                rating -= 999999999;
            }
            return rating + status_rating;
        }
    };
    struct TetrisNode
    {
        double rating;
        std::size_t version;
        TetrisNode *parent;
        TetrisMap map;
        TetrisStatus status;
        std::vector<TetrisNode *> children;
        TetrisNode(std::size_t version, TetrisNode *parent, TetrisMap &map, TetrisStatus &status) : rating(0), version(version), parent(parent), map(map), status(status)
        {
        }
        TetrisNode(double &rating, std::size_t &version, TetrisNode *parent, TetrisMap &map, TetrisStatus &status) : rating(rating), version(version), parent(parent), map(map), status(status)
        {
        }
        double calc_rating(std::string &path)
        {
            if (parent != nullptr)
            {
                if (version == 1)
                {
                    path = status.next.active.path;
                }
                return parent->calc_rating(path) + rating;
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
        std::unordered_map<int, bool> visited_db;
        std::unordered_map<uint32_t, bool> result_db;
        std::vector<std::function<bool(TetrisActive &)>> moves;
        TetrisInstructor instructor;
        TetrisConfig &config;
        int build_hash(const TetrisCoord &coord) const
        {
            return coord.x + coord.y * 100 + coord.r * 10000;
        }
        TetrisPathManager(TetrisActive &active, TetrisConfig &config, TetrisMap &map) : instructor(map, active.type), config(config)
        {
            std::size_t size = map.width * map.height * 4;
            visited_db.reserve(size);
            visited_db[build_hash(active)] = true;
            result_db.reserve(static_cast<std::size_t>(size / 2));
            search.push(active);

            if (config.allow_D)
                moves.push_back([this](TetrisActive &a)
                                { return instructor.D(a); });
            if (config.allow_d)
                moves.push_back([this](TetrisActive &a)
                                { return instructor.d(a); });
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
            if (config.allow_x)
                moves.push_back([this](TetrisActive &a)
                                { return instructor.x(a); });
            moves.push_back([this](TetrisActive &a)
                            { return instructor.c(a); });
            moves.push_back([this](TetrisActive &a)
                            { return instructor.z(a); });
        }

        std::vector<TetrisActive> test_run()
        {
            TetrisActive temp;
            std::vector<TetrisActive> result;
            result.reserve(1000);
            while (!search.empty())
            {
                auto &current = search.front();
                for (auto &move : moves)
                {
                    temp = current;
                    if (move(temp))
                    {
                        int hash = build_hash(temp);
                        if (!visited_db[hash])
                        {
                            visited_db[hash] = true;
                            search.push(temp);
                        }
                    }
                }

                instructor.build_snaphash(current);
                if (!result_db[current.snapshot])
                {
                    result_db[current.snapshot] = true;
                    result.push_back(current);
                }
                search.pop();
            }
            return result;
        }
        TetrisActive run_lite(TetrisMap map, TetrisStatus status, TetrisParam &p)
        {
            double best = std::numeric_limits<double>::lowest();
            TetrisActive best_active;
            TetrisActive temp;
            TetrisEvaluation test(p);
            while (!search.empty())
            {
                auto &current = search.front();
                for (auto &move : moves)
                {
                    temp = current;
                    if (move(temp))
                    {
                        int hash = build_hash(temp);
                        if (!visited_db[hash])
                        {
                            visited_db[hash] = true;
                            search.push(temp);
                        }
                    }
                }

                instructor.build_snaphash(current);
                if (!result_db[current.snapshot])
                {
                    result_db[current.snapshot] = true;
                    TetrisMap map_copy = map;
                    TetrisStatus status_copy = status;
                    status_copy.next.active = current;
                    status_copy.allspin = instructor.immobile(current);
                    instructor.attach(map_copy, current);
                    status_copy.clear = map_copy.flush();
                    double rating = test.begin_judgement(status, status_copy, map_copy, 1, instructor, config);
                    if (rating > best)
                    {
                        best = rating;
                        best_active = current;
                    }
                }
                search.pop();
            }
            return best_active;
        }
        void run(TetrisNode *node, TetrisNextManager &next_manager, TetrisParam &p)
        {
            TetrisActive temp;
            TetrisMap map_cache;
            TetrisStatus status_cache;
            TetrisEvaluation test(p);
            double rating_ = 0;
            std::size_t new_version = node->version + 1;
            while (!search.empty())
            {
                auto &current = search.front();

                for (auto &move : moves)
                {
                    temp = current;
                    if (move(temp))
                    {
                        int hash = build_hash(temp);
                        if (!visited_db[hash])
                        {
                            visited_db[hash] = true;
                            search.push(temp);
                        }
                    }
                }

                instructor.build_snaphash(current);
                if (!result_db[current.snapshot])
                {
                    result_db[current.snapshot] = true;
                    map_cache = node->map;
                    status_cache = node->status;
                    status_cache.next = next_manager;
                    status_cache.next.active = current;
                    status_cache.allspin = instructor.immobile(current);
                    instructor.attach(map_cache, current);
                    status_cache.clear = map_cache.flush();
                    rating_ = test.begin_judgement(node->status, status_cache, map_cache, new_version, instructor, config);
                    node->children.emplace_back(new TetrisNode(rating_, new_version, node, map_cache, status_cache));
                }
                search.pop();
            }
        }
    };
    using TetrisNodeResult = std::pair<double, std::string>;
    struct TetrisTree
    {
        TetrisNode *root;
        std::queue<TetrisNode *> tasks;
        TetrisConfig &config;
        TetrisParam &param;
        std::size_t stable_version;
        TetrisNodeResult stable, beta, alpha;
        TetrisNextManager next_manager;
        std::size_t total_nodes;
        double low;
        TetrisTree(TetrisMap &map, TetrisStatus &status, TetrisConfig &config, TetrisParam &param) : config(config), param(param), next_manager(config), total_nodes(0)
        {
            stable_version = 0;
            root = new TetrisNode(stable_version, nullptr, map, status);
            tasks.push(root);
            low = stable.first = beta.first = std::numeric_limits<double>::lowest();
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
            double &min_rating = node->children.front()->rating;
            double max_rating = node->children.back()->rating;
            double prune_threshold = 0.5 * (max_rating - min_rating);
            auto it = node->children.rbegin();
            while (node->children.size() > 5 && it != node->children.rend() && (max_rating - (*it)->rating > prune_threshold))
            {
                delete *it;
                it = decltype(it)(node->children.erase(std::next(it).base()));
            }
            for (const auto &child : node->children)
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
                beta.first = low;
            }
            alpha.first = node->calc_rating(alpha.second);
            if (!node->children.empty())
            {
                const TetrisNode *first_child = node->children[0];
                alpha.first += first_child->rating;
                if (stable_version == 1)
                {
                    alpha.second = first_child->status.next.active.path;
                }
            }
            if (alpha.first > beta.first)
            {
                beta = alpha;
            }
        }
        void expand(TetrisNode *node)
        {
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
                path.run(node, next_manager, param);
            }
            if (config.can_hold && !node->status.next.changed_hold)
            {
                if (next_manager.change_hold())
                {
                    TetrisPathManager path(next_manager.active, config, node->map);
                    path.run(node, next_manager, param);
                }
            }
            total_nodes += node->children.size();
            update_task(node);
            compare_and_update(node);
        }
        std::string run()
        {
            auto now = std::chrono::high_resolution_clock::now();
            while ((stable.first == low ||
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
    struct TetrisGameEmulation
    {
        void run(TetrisMap &map, TetrisNextManager &next, TetrisStatus &status, std::string &path)
        {
            if (path.front() == 'v')
            {
                next.change_hold();
            }
            status.next = next;
            TetrisInstructor move(map, status.next.active.type);
            for (auto &c : path)
            {
                switch (c)
                {
                case 'D':
                    move.D(status.next.active);
                    break;
                case 'd':
                    move.consumer_d(status.next.active);
                    break;
                case 'L':
                    move.L(status.next.active);
                    break;
                case 'l':
                    move.l(status.next.active);
                    break;
                case 'R':
                    move.R(status.next.active);
                    break;
                case 'r':
                    move.r(status.next.active);
                    break;
                case 'x':
                    move.x(status.next.active);
                    break;
                case 'c':
                    move.c(status.next.active);
                    break;
                case 'z':
                    move.z(status.next.active);
                    break;
                }
            }
            status.allspin = move.immobile(status.next.active);
            move.attach(map, status.next.active);
            status.clear = map.flush();
            status.attack = 0;
            switch (status.clear)
            {
            case 0:
                status.combo = 0;
                if (status.garbage.estimate_damage() > 0)
                status.garbage.take_all_damage(map, atk.messiness);
                status.garbage.decay();
                break;
            case 1:
                if (status.allspin)
                {
                    status.attack += atk.ass + status.b2b;
                    status.b2b = atk.b2b;
                }
                else
                {
                    status.b2b = 0;
                    status.attack += atk.single;
                }
                status.attack += atk.combo_table[++status.combo];
                break;
            case 2:
                if (status.allspin)
                {
                    status.attack += atk.asd + status.b2b;
                    status.b2b = atk.b2b;
                }
                else
                {
                    status.b2b = 0;
                    status.attack += atk.double_;
                }
                status.attack += atk.combo_table[++status.combo];
                break;
            case 3:
                if (status.allspin)
                {
                    status.attack += atk.ast + status.b2b;
                    status.b2b = atk.b2b;
                }
                else
                {
                    status.b2b = 0;
                    status.attack += atk.triple;
                }
                status.attack += atk.combo_table[++status.combo];
                break;
            case 4:
                status.attack += atk.quad + status.b2b + atk.combo_table[++status.combo];
                status.b2b = atk.b2b;
                break;
            }
            if (!map.roof)
            {
                status.attack = atk.pc;
            }
            status.send_attack = status.attack;
            status.garbage.fight_lines(status.send_attack);
            TetrisActive next_mino(next.config.default_x, next.config.default_y, next.config.default_r, status.next.queue.front());
            status.dead = move.check_death(map, next_mino);
            status.allspin_value = 0;
            TetrisEvaluation::find_every_spin(map, status.allspin_value);
        }
    };
}