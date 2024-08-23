#pragma once
#include <cstdint>
#include <random>
#include <map>
#include <unordered_set>
#include <fstream>
#include <string>
#include <queue>
#include <vector>
#include <algorithm>
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
        TetrisCoord(const int8_t &x, const int8_t &y, const int8_t &r) : x(x), y(y), r(r) {}
        bool operator==(const TetrisCoord &other) const
        {
            return x == other.x && y == other.y && r == other.r;
        }
    };
    struct TetrisCoordHash
    {
        std::size_t operator()(const TetrisCoord &coord) const
        {
            std::size_t h1 = std::hash<int8_t>{}(coord.x);
            std::size_t h2 = std::hash<int8_t>{}(coord.y);
            std::size_t h3 = std::hash<int8_t>{}(coord.r);

            std::size_t combined = h1 * 31 + h2;
            combined = combined * 31 + h3;

            return combined;
        }
    };

    struct TetrisActive : public TetrisCoord
    {
        uint8_t type;
        bool last_rotate;
        int8_t last_kick;
        std::string path;
        uint32_t snapshot[4];
        bool operator==(const uint32_t other[4]) const
        {
            return std::memcmp(snapshot, other, sizeof(snapshot)) == 0;
        }
        void operator=(const TetrisActive &other)
        {
            memcpy(this, &other, sizeof(TetrisActive));
        }
        TetrisActive() : TetrisCoord(), type(EMPTY), last_rotate(false), last_kick(-1) {}
        TetrisActive(const int8_t &x, const int8_t &y, const int8_t &r, const uint8_t &type) : TetrisCoord(x, y, r), type(type), last_rotate(false), last_kick(-1) {}
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
    using TetrisMinotypes = std::unordered_map<TetrisMinoType, TetrisMino>;
    using TetrisMinocache = std::unordered_map<TetrisMinoType, std::vector<std::unordered_map<int8_t, uint32_t[4]>>>;
    using TetrisMinocacheMini = std::vector<std::unordered_map<int8_t, uint32_t[4]>>;
    struct TetrisMinoManager
    {
        TetrisMinotypes mino_list;
        TetrisMinocache move_cache;
        TetrisMinotypes &get()
        {
            return mino_list;
        }
        TetrisMinocache &get_move_cache()
        {
            return move_cache;
        }
        TetrisMinoManager(const std::string &path)
        {
            json jsondata;
            std::ifstream file(path);
            file >> jsondata;
            file.close();
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
                        std::vector<std::pair<int8_t, int8_t>> kick;
                        for (int8_t k = 0; k < data.size(); k++)
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
                        for (int8_t k = 0; k < data.size(); k++)
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
                        for (int8_t k = 0; k < data.size(); k++)
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
                    auto &data = move_cache[char_to_type[type]];
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
                        data.push_back(moves);
                    }
                }
            }
            catch (std::exception &e)
            {
                printf("%s\n", e.what());
                throw std::runtime_error("An error occurred while parsing the mino file, please check your file configuration.");
            }
        }
    };
    struct TetrisInstructor
    {
        TetrisMap &map;
        TetrisMinocacheMini &move_cache;
        TetrisMino &mino;
        TetrisInstructor(TetrisMap &map, TetrisMinocacheMini &move_cache, TetrisMino &mino) : map(map), move_cache(move_cache), mino(mino) {
            map.scan();
        }
        bool l(TetrisActive &active)
        {
            active.x -= 1;
            if (active.x < mino.left_offset[active.r])
            {
                active.x += 1;
                return false;
            }
            if (active.y >= map.roof)
            {
                active.path += 'l';
                active.last_rotate = false;
                return true;
            }
            if (map.board[active.y + 0] & move_cache[active.r][active.x][0] ||
                map.board[active.y + 1] & move_cache[active.r][active.x][1] ||
                map.board[active.y + 2] & move_cache[active.r][active.x][2] ||
                map.board[active.y + 3] & move_cache[active.r][active.x][3])
            {
                active.x += 1;
                return false;
            }
            active.last_rotate = false;
            active.path += 'l';
            return true;
        }
        bool r(TetrisActive &active)
        {
            active.x += 1;
            if (active.x > map.width - mino.right_offset[active.r] - 4)
            {
                active.x -= 1;
                return false;
            }
            if (active.y >= map.roof)
            {
                active.path += 'r';
                active.last_rotate = false;
                return true;
            }
            if (map.board[active.y + 0] & move_cache[active.r][active.x][0] ||
                map.board[active.y + 1] & move_cache[active.r][active.x][1] ||
                map.board[active.y + 2] & move_cache[active.r][active.x][2] ||
                map.board[active.y + 3] & move_cache[active.r][active.x][3])
            {
                active.x -= 1;
                return false;
            }
            active.path += 'r';
            active.last_rotate = false;
            return true;
        }
        bool L(TetrisActive &active)
        {
            if (active.x == mino.left_offset[active.r])
                return false;
            if (active.y >= map.roof)
            {
                active.x = mino.left_offset[active.r];
                active.last_rotate = false;
                active.path += 'L';
                return true;
            }
            int count = 0;
            for (; active.x > mino.left_offset[active.r]; active.x--, ++count)
            {
                if (map.board[active.y + 0] & move_cache[active.r][active.x][0] ||
                    map.board[active.y + 1] & move_cache[active.r][active.x][1] ||
                    map.board[active.y + 2] & move_cache[active.r][active.x][2] ||
                    map.board[active.y + 3] & move_cache[active.r][active.x][3])
                {
                    ++active.x;
                    break;
                }
            }
            if (count)
            {
                active.last_rotate = false;
                active.path += 'L';
            }
            return count;
        }
        bool R(TetrisActive &active)
        {
            int8_t rightmost = map.width - mino.right_offset[active.r] - 4;
            if (active.x == rightmost)
                return false;
            if (active.y >= map.roof)
            {
                active.x = rightmost;
                active.last_rotate = false;
                active.path += 'R';
                return true;
            }
            int count = 0;
            for (; active.x < rightmost; active.x++, ++count)
            {
                if (map.board[active.y + 0] & move_cache[active.r][active.x][0] ||
                    map.board[active.y + 1] & move_cache[active.r][active.x][1] ||
                    map.board[active.y + 2] & move_cache[active.r][active.x][2] ||
                    map.board[active.y + 3] & move_cache[active.r][active.x][3])
                {
                    active.x -= 1;
                    break;
                }
            }
            if (count)
            {
                active.last_rotate = false;
                active.path += 'R';
            }
            return count;
        }
        bool d(TetrisActive &active)
        {
            if (active.y >= map.roof)
            {
                active.path.append(active.y - map.roof, 'd');
                active.y = map.roof;
                active.last_rotate = false;
                return true;
            }
            active.y -= 1;
            if (active.y < mino.down_offset[active.r] ||
                map.board[active.y + 0] & move_cache[active.r][active.x][0] ||
                map.board[active.y + 1] & move_cache[active.r][active.x][1] ||
                map.board[active.y + 2] & move_cache[active.r][active.x][2] ||
                map.board[active.y + 3] & move_cache[active.r][active.x][3])
            {
                active.y += 1;
                return false;
            }
            active.last_rotate = false;
            active.path += 'd';
            return true;
        }
        bool D(TetrisActive &active)
        {
            if (active.y == mino.down_offset[active.r])
                return false;
            int count = 0;
            if (active.y >= map.roof)
            {
                active.y = map.roof;
                active.last_rotate = false;
                ++count;
            }
            for (; active.y > mino.down_offset[active.r]; active.y--, ++count)
            {
                if (map.board[active.y + 0] & move_cache[active.r][active.x][0] ||
                    map.board[active.y + 1] & move_cache[active.r][active.x][1] ||
                    map.board[active.y + 2] & move_cache[active.r][active.x][2] ||
                    map.board[active.y + 3] & move_cache[active.r][active.x][3])
                {
                    active.y += 1;
                    break;
                }
            }
            if (count)
            {
                active.last_rotate = false;
                active.path += 'D';
            }
            return count;
        }
        void D_PRESERVE_LAST_ROTATE(TetrisActive &active)
        {
            if (active.y == mino.down_offset[active.r])
                return;
            int count = 0;
            if (active.y >= map.roof)
            {
                active.y = map.roof;
                ++count;
            }
            for (; active.y > mino.down_offset[active.r]; active.y--, ++count)
            {
                if (map.board[active.y + 0] & move_cache[active.r][active.x][0] ||
                    map.board[active.y + 1] & move_cache[active.r][active.x][1] ||
                    map.board[active.y + 2] & move_cache[active.r][active.x][2] ||
                    map.board[active.y + 3] & move_cache[active.r][active.x][3])
                {
                    active.y += 1;
                    return;
                }
            }
            return;
        }
        bool c(TetrisActive &active)
        {
            active.r = (active.r + 1) & 3;
            int8_t rightmost = map.width - mino.right_offset[active.r] - 4;
            if (active.x >= mino.left_offset[active.r] && active.x <= rightmost && active.y >= mino.down_offset[active.r])
            {
                if (!(map.board[active.y + 0] & move_cache[active.r][active.x][0] ||
                      map.board[active.y + 1] & move_cache[active.r][active.x][1] ||
                      map.board[active.y + 2] & move_cache[active.r][active.x][2] ||
                      map.board[active.y + 3] & move_cache[active.r][active.x][3]))
                {
                    active.path += 'c';
                    active.last_rotate = true;
                    active.last_kick = -1;
                    return true;
                }
            }
            for (int i = 0; i < mino.rotate_right[active.r].size(); i++)
            {
                int8_t x = active.x + mino.rotate_right[active.r][i].first;
                int8_t y = active.y + mino.rotate_right[active.r][i].second;
                if (x < mino.left_offset[active.r] || x > rightmost || y < mino.down_offset[active.r])
                    continue;
                if (!(map.board[y + 0] & move_cache[active.r][x][0] ||
                      map.board[y + 1] & move_cache[active.r][x][1] ||
                      map.board[y + 2] & move_cache[active.r][x][2] ||
                      map.board[y + 3] & move_cache[active.r][x][3]))
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
        bool z(TetrisActive &active)
        {
            active.r = (active.r - 1) & 3;
            int8_t rightmost = map.width - mino.right_offset[active.r] - 4;
            if (active.x >= mino.left_offset[active.r] && active.x <= rightmost && active.y >= mino.down_offset[active.r])
            {
                if (!(map.board[active.y + 0] & move_cache[active.r][active.x][0] ||
                      map.board[active.y + 1] & move_cache[active.r][active.x][1] ||
                      map.board[active.y + 2] & move_cache[active.r][active.x][2] ||
                      map.board[active.y + 3] & move_cache[active.r][active.x][3]))
                {
                    active.path += 'z';
                    active.last_rotate = true;
                    active.last_kick = -1;
                    return true;
                }
            }
            for (int i = 0; i < mino.rotate_left[active.r].size(); i++)
            {
                int8_t x = active.x + mino.rotate_left[active.r][i].first;
                int8_t y = active.y + mino.rotate_left[active.r][i].second;
                if (x < mino.left_offset[active.r] || x > rightmost || y < mino.down_offset[active.r])
                    continue;
                if (!(map.board[y + 0] & move_cache[active.r][x][0] ||
                      map.board[y + 1] & move_cache[active.r][x][1] ||
                      map.board[y + 2] & move_cache[active.r][x][2] ||
                      map.board[y + 3] & move_cache[active.r][x][3]))
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
        bool x(TetrisActive &active)
        {
            active.r = (active.r + 2) & 3;
            int8_t rightmost = map.width - mino.right_offset[active.r] - 4;
            if (active.x >= mino.left_offset[active.r] && active.x <= rightmost && active.y >= mino.down_offset[active.r])
            {
                if (!(map.board[active.y + 0] & move_cache[active.r][active.x][0] ||
                      map.board[active.y + 1] & move_cache[active.r][active.x][1] ||
                      map.board[active.y + 2] & move_cache[active.r][active.x][2] ||
                      map.board[active.y + 3] & move_cache[active.r][active.x][3]))
                {
                    active.path += 'x';
                    active.last_rotate = true;
                    active.last_kick = -1;
                    return true;
                }
            }
            for (int i = 0; i < mino.rotate_180[active.r].size(); i++)
            {
                int8_t x = active.x + mino.rotate_180[active.r][i].first;
                int8_t y = active.y + mino.rotate_180[active.r][i].second;
                if (x < mino.left_offset[active.r] || x > rightmost || y < mino.down_offset[active.r])
                    continue;
                if (!(map.board[y + 0] & move_cache[active.r][x][0] ||
                      map.board[y + 1] & move_cache[active.r][x][1] ||
                      map.board[y + 2] & move_cache[active.r][x][2] ||
                      map.board[y + 3] & move_cache[active.r][x][3]))
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
        void build_snapshot(TetrisActive &active)
        {
            TetrisActive copy = active;
            D_PRESERVE_LAST_ROTATE(copy);
            memset(active.snapshot, 0, sizeof(active.snapshot));
            int8_t by = active.y - mino.down_offset[active.r];
            for (int i = -mino.down_offset[active.r]; i < 4; i++)
            {
                active.snapshot[3 - i] = map.board[by + i] | move_cache[active.r][active.x][i + mino.down_offset[active.r]];
            }
        }
    };
    struct TetrisPathManager
    {
        std::queue<TetrisActive> search;
        std::unordered_set<TetrisCoord, TetrisCoordHash> visited;
        std::vector<TetrisActive> result;
        TetrisInstructor instructor;
        TetrisConfig &config;
        bool change_hold;
        TetrisPathManager(TetrisActive &active, TetrisConfig &config, TetrisMap &map, TetrisMinocacheMini &move_cache, TetrisMino &mino) : instructor(map, move_cache, mino), config(config), change_hold(false)
        {
            search.push(active);
            visited.insert(active);
        }
        void run()
        {
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
                TetrisActive current = search.front();
                search.pop();

                for (auto &move : moves)
                {
                    TetrisActive temp = current;
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
    };
}