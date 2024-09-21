#pragma once
#include "board.hpp"
#include "next.hpp"
#include "mino.hpp"
#include "pending.hpp"
#include "minotemplate.h"

namespace moenew
{
    class Evaluation
    {
    public:
        struct AttackTable
        {
            int clear_1;
            int clear_2;
            int clear_3;
            int clear_4;
            int aspin_1;
            int aspin_2;
            int aspin_3;
            int aspin_4;
            int b2b;
            int pc;
            double multiplier;
            double messiness;
            int combo[21];
            int get_combo(int c) const
            {
                if (c > 20)
                {
                    return combo[20];
                }
                if (c < 0)
                {
                    return combo[0];
                }
                return combo[c];
            }
        };
        struct Status
        {
            double rating;
            int clear;
            int combo;
            int attack;
            int send_attack;
            int cumulative_attack;
            int attack_since;
            bool allspin;
            bool dead;
            bool b2b;
            Pending under_attack;
            Next next;
            Board board;
            void reset()
            {
                rating = 0;
                clear = 0;
                combo = 0;
                attack = 0;
                send_attack = 0;
                cumulative_attack = 0;
                attack_since = 0;
                allspin = false;
                dead = false;
                b2b = false;
                under_attack.lines.clear();
                next.reset();
                board.clear();
            }
        };
        enum Param
        {
            CLEAR_1,
            CLEAR_2,
            CLEAR_3,
            CLEAR_4,
            ASPIN_1,
            ASPIN_2,
            ASPIN_3,
            ASPIN_SLOT,
            B2B,
            COMBO,
            ATTACK,
            CANCEL,
            TANK_CLEAN,
            HEIGHT,
            COL_TRANS,
            ROW_TRANS,
            HOLE_COUNT,
            HOLE_LINE,
            WIDE_2,
            WIDE_3,
            WIDE_4,
            BUILD_ATTACK,
            SPIKE,
            PENDING_LOCK,
            PENDING_HOLD,
            END_OF_PARAM
        };
        struct Playstyle
        {
            double param[64];
            constexpr double &operator[](int i)
            {
                return param[i];
            }
            Playstyle()
            {
                memset(param, 0, sizeof(param));
                // param[COL_TRANS] = 1;
                // param[ROW_TRANS] = 1;
                // param[HOLE_COUNT] = -1;
                // param[HOLE_LINE] = 1;
                // param[WIDE_2] = 1;
                // param[WIDE_3] = 1;
                // param[WIDE_4] = 1;
                // param[BUILD_ATTACK] = 1;
                // param[SPIKE] = 5;
            }
            bool operator==(const Playstyle &rhs) const
            {
                return memcmp(param, rhs.param, sizeof(param)) == 0;
            }
        };
        Playstyle p;
        AttackTable atk;
        static void find_every_spin(const Board &board, int &val)
        {
            for (int y = board.y_max; y >= std::max<int>(0, board.y_max - 4); --y)
            {
                int rowm1 = board.field[y - 1];
                int row0 = board.field[y];
                int row1 = board.field[y + 1];
                int row2 = board.field[y + 2];
                int row3 = board.field[y + 3];
                int row4 = board.field[y + 4];
                int count0 = std::popcount(board.field[y]);
                int count1 = std::popcount(board.field[y + 1]);
                int count2 = std::popcount(board.field[y + 2]);
                for (int x = 0; x < board.w; ++x)
                {
                    int xm1 = x - 1;
                    int x1 = x + 1;
                    int x2 = x + 2;
                    int x3 = x + 3;
                    int x4 = x + 4;
                    if (x < board.w - 2)
                    {
                        // S spin
                        if (~row0 & loc_x.of(x) && ~row0 & loc_x.of(x1) && ~row1 & loc_x.of(x1) && ~row1 & loc_x.of(x2))
                        {
                            // double/single
                            if ((row0 & loc_x.of(x2) || (y == 0 || rowm1 & loc_x.of(x1))) && (row1 & loc_x.of(x) || (row2 & loc_x.of(x2) && (x == 0 || row0 & loc_x.of(xm1)))))
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
                        if (~row2 & loc_x.of(x) && ~row1 & loc_x.of(x) && ~row1 & loc_x.of(x1) && ~row0 & loc_x.of(x1))
                        {
                            // triple
                            if (row0 & loc_x.of(x) && (row2 & loc_x.of(x1) || (row0 & loc_x.of(x2) && row3 & loc_x.of(x))))
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
                        if (~row1 & loc_x.of(x) && ~row1 & loc_x.of(x1) && ~row0 & loc_x.of(x1) && ~row0 & loc_x.of(x2))
                        {
                            // double/single
                            if ((row0 & loc_x.of(x) || (y == 0 || rowm1 & loc_x.of(x))) && (row1 & loc_x.of(x2) || (row2 & loc_x.of(x) && (x3 == board.w || row0 & loc_x.of(x3)))))
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
                        if (~row2 & loc_x.of(x1) && ~row1 & loc_x.of(x1) && ~row1 & loc_x.of(x) && ~row0 & loc_x.of(x))
                        {
                            // triple
                            if (row0 & loc_x.of(x1) && (row2 & loc_x.of(x) || ((x != 0 && row0 & loc_x.of(xm1)) && row3 & loc_x.of(x1))))
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
                        if (~row0 & loc_x.of(x) && ~row0 & loc_x.of(x1) && ~row0 & loc_x.of(x2) && ~row1 & loc_x.of(x2))
                        {
                            // double
                            bool cond1 = row1 & loc_x.of(x1) && (x3 == board.w || row0 & loc_x.of(x3) || row1 & loc_x.of(x3));
                            bool cond2 = (x3 == board.w || row1 & loc_x.of(x3)) && row1 & loc_x.of(x);
                            bool cond3 = y == 0 || rowm1 & loc_x.of(x) || rowm1 & loc_x.of(x1) || rowm1 & loc_x.of(x2);
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
                        if (~row0 & loc_x.of(x) && ~row1 & loc_x.of(x) && ~row1 & loc_x.of(x1) && ~row1 & loc_x.of(x2))
                        {
                            // double, 180 facing
                            bool cond1 = (x == 0 || row1 & loc_x.of(xm1)) && row0 & loc_x.of(x1) && row2 & loc_x.of(x2);
                            bool cond2 = x != 0 && row0 & loc_x.of(xm1) && row0 & loc_x.of(x1) && row2 & loc_x.of(x1);
                            bool cond3 = (x == 0 || row1 & loc_x.of(xm1) || row0 & loc_x.of(xm1)) && row0 & loc_x.of(x1) && row2 & loc_x.of(x1);
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
                        if (~row0 & loc_x.of(x) && ~row1 & loc_x.of(x) && ~row2 & loc_x.of(x) && ~row0 & loc_x.of(x1))
                        {
                            // triple
                            bool cond1 = x != 0 || row0 & loc_x.of(xm1) || row1 & loc_x.of(xm1) || row2 & loc_x.of(xm1);
                            bool cond2 = row1 & loc_x.of(x1) || (row3 & loc_x.of(x) && row0 & loc_x.of(x2));
                            bool cond3 = y == 0 || rowm1 & loc_x.of(x) || rowm1 & loc_x.of(x1);
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
                        if (~row0 & loc_x.of(x) && ~row0 & loc_x.of(x1) && ~row0 & loc_x.of(x2) && ~row1 & loc_x.of(x))
                        {
                            // double
                            bool cond1 = row1 & loc_x.of(x1) && (x == 0 || row0 & loc_x.of(xm1) || row1 & loc_x.of(xm1));
                            bool cond2 = (x == 0 || row1 & loc_x.of(xm1)) && row1 & loc_x.of(x2);
                            bool cond3 = y == 0 || rowm1 & loc_x.of(x) || rowm1 & loc_x.of(x1) || rowm1 & loc_x.of(x2);
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
                        if (~row1 & loc_x.of(x) && ~row1 & loc_x.of(x1) && ~row1 & loc_x.of(x2) && ~row0 & loc_x.of(x2))
                        {
                            // double, 180 facing
                            bool cond1 = (x3 == board.w || row1 & loc_x.of(x3)) && row0 & loc_x.of(x1) && row2 & loc_x.of(x);
                            bool cond2 = x3 != board.w && row0 & loc_x.of(x3) && row0 & loc_x.of(x1) && row2 & loc_x.of(x1);
                            bool cond3 = (x3 == board.w || row1 & loc_x.of(x3) || row0 & loc_x.of(x3)) && row0 & loc_x.of(x1) && row2 & loc_x.of(x1);
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
                        if (~row0 & loc_x.of(x) && ~row0 & loc_x.of(x1) && ~row1 & loc_x.of(x1) && ~row2 & loc_x.of(x1))
                        {
                            // triple
                            bool cond1 = row0 & loc_x.of(x2) || row1 & loc_x.of(x2) || row2 & loc_x.of(x2);
                            bool cond2 = row1 & loc_x.of(x);
                            bool cond3 = y == 0 || rowm1 & loc_x.of(x) || rowm1 & loc_x.of(x1);
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
                        if (~row0 & loc_x.of(x1) && ~row1 & loc_x.of(x) && ~row1 & loc_x.of(x1) && ~row1 & loc_x.of(x2) && ~row2 & loc_x.of(x1))
                        {
                            // double
                            if (row0 & loc_x.of(x) && row0 & loc_x.of(x2) && (row2 & loc_x.of(x) || row2 & loc_x.of(x2)))
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
                        if (~row0 & loc_x.of(x1) && ~row1 & loc_x.of(x1) && ~row2 & loc_x.of(x1))
                        {
                            if ((~row1 & loc_x.of(x) && row1 & loc_x.of(x2)) || (row1 & loc_x.of(x) && ~row1 & loc_x.of(x2)))
                            {
                                // triple
                                if (row0 & loc_x.of(x) && row0 & loc_x.of(x2) && row2 & loc_x.of(x) && row2 & loc_x.of(x2))
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
                        if (~row0 & loc_x.of(x) && ~row0 & loc_x.of(x1) && ~row0 & loc_x.of(x2) && ~row1 & loc_x.of(x1))
                        {
                            // single (mini)
                            bool cond1 = row1 & loc_x.of(x) && (x3 == board.w || row0 & loc_x.of(x3));
                            bool cond2 = row1 & loc_x.of(x2) && (x == 0 || row0 & loc_x.of(xm1));
                            bool cond3 = y == 0 || rowm1 & loc_x.of(x) || rowm1 & loc_x.of(x1) || rowm1 & loc_x.of(x2);
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
                    if (x < board.w - 3 && ~row0 & loc_x.of(x) && ~row0 & loc_x.of(x1) && ~row0 & loc_x.of(x2) && ~row0 & loc_x.of(x3))
                    {
                        // single
                        bool up_cover = row1 & loc_x.of(x) || row1 & loc_x.of(x1) || row1 & loc_x.of(x2) || row1 & loc_x.of(x3);
                        if ((x == 0 || row0 & loc_x.of(xm1)) && (x4 == board.w || row0 & loc_x.of(x4)) && up_cover)
                        {
                            ++val;
                            if (count0 == 6)
                            {
                                val += 2;
                            }
                        }
                    }
                    if (~row0 & loc_x.of(x) && ~row1 & loc_x.of(x) && ~row2 & loc_x.of(x) && ~row3 & loc_x.of(x))
                    {
                        // triple
                        bool left_cover = x == 0 || row0 & loc_x.of(xm1) || row1 & loc_x.of(xm1) || row2 & loc_x.of(xm1) || row3 & loc_x.of(xm1);
                        bool right_cover = x1 == board.w || row0 & loc_x.of(x1) || row1 & loc_x.of(x1) || row2 & loc_x.of(x1) || row3 & loc_x.of(x1);
                        if (row4 & loc_x.of(x) && left_cover && right_cover && (y == 0 || rowm1 & loc_x.of(x)))
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
                    if (~row0 & loc_x.of(x) && ~row0 & loc_x.of(x1) && ~row1 & loc_x.of(x) && ~row1 & loc_x.of(x1))
                    {
                        bool down_cover = y == 0 || rowm1 & loc_x.of(x) || rowm1 & loc_x.of(x1);
                        bool left_cover = x == 0 || row0 & loc_x.of(xm1) || row1 & loc_x.of(xm1);
                        bool right_cover = x2 == board.w || (x2 < board.w && (row0 & loc_x.of(x2) || row1 & loc_x.of(x2)));
                        bool up_cover = row2 & loc_x.of(x) || row2 & loc_x.of(x1);
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
        // Level 1: Evaluate the board
        // Level 2: Evaluate line clear, consequences of accepting garbage
        // Level 3: Evaluate allspin setups, wasted, held minos, and other misc stuff
        // Pruning is done each time the decision is evaluated
        void evaluation_level_1(const Status &last, Status &ret, int depth)
        {
            struct
            {
                int col_trans;
                int row_trans;
                int hole_count;
                int hole_line;
                int wide[32];
            } e;
            memset(&e, 0, sizeof(e));
            const auto &board = ret.board;
            for (int y = board.y_max; y >= 0; y--)
            {
                int wide_max = 0;
                int wide_count = 0;
                for (int x = 0; x < board.w; x++)
                {
                    if (board.get(x, y))
                    {
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
                    if (x + 1 != board.w)
                    {
                        e.row_trans += board.get(x, y) != board.get(x + 1, y);
                    }
                }
                if (std::popcount(board.field[y]) == board.w - wide_max)
                {
                    e.wide[wide_max]++;
                }
                if (y - 1 >= 0)
                {
                    int check = e.hole_count;
                    e.hole_count += std::popcount(board.field[y] & ~board.field[y - 1]);
                    e.hole_line += check != e.hole_count;
                    e.col_trans += std::popcount(board.field[y] ^ board.field[y - 1]);
                }
            }
            if (!ret.next.next.empty())
            {
                const uint32_t *mino = cache_get(ret.next.peek(), DEFAULT_R, DEFAULT_X);
                if (!ret.board.integrate(mino, DEFAULT_Y))
                {
                    ret.dead = true;
                }
            }
            ret.rating = (0. - p[HEIGHT] * board.y_max - p[COL_TRANS] * e.col_trans - p[ROW_TRANS] * e.row_trans - p[HOLE_COUNT] * e.hole_count - p[HOLE_LINE] * e.hole_line + p[WIDE_2] * e.wide[2] + p[WIDE_3] * e.wide[3] + p[WIDE_4] * e.wide[4] - 999999 * ret.dead);
        }
        void evaluation_level_2(const Status &last, Status &ret, int depth)
        {
            double like = 0;
            ret.attack = 0;
            switch (ret.clear)
            {
            case 0:
                like += (ret.under_attack.estimate_mess() - 4) * p[TANK_CLEAN];
                ret.under_attack.accept(ret.board, atk.messiness);
                ret.under_attack.decay();
                if (!ret.under_attack.lines.empty())
                {
                    like += ret.under_attack.lines[0].delay == 0 ? (ret.under_attack.estimate() * (ret.under_attack.estimate() - last.combo)) * p[PENDING_LOCK] : 0;
                }
                ret.combo = 0;
                break;
            case 1:
                if (ret.allspin)
                {
                    ret.attack = atk.aspin_1 + ret.b2b;
                    ret.b2b = true;
                    like += p[ASPIN_1];
                }
                else
                {
                    ret.attack = atk.clear_1;
                    ret.b2b = false;
                    like += p[CLEAR_1];
                }
                ret.attack += atk.get_combo(++ret.combo);
                break;
            case 2:
                if (ret.allspin)
                {
                    ret.attack = atk.aspin_2 + ret.b2b;
                    ret.b2b = true;
                    like += p[ASPIN_2];
                }
                else
                {
                    ret.attack = atk.clear_2;
                    ret.b2b = false;
                    like += p[CLEAR_2];
                }
                ret.attack += atk.get_combo(++ret.combo);
                break;
            case 3:
                if (ret.allspin)
                {
                    ret.attack = atk.aspin_3 + ret.b2b;
                    ret.b2b = true;
                    like += p[ASPIN_3];
                }
                else
                {
                    ret.attack = atk.clear_3;
                    ret.b2b = false;
                    like += p[CLEAR_3];
                }
                ret.attack += atk.get_combo(++ret.combo);
                break;
            case 4:
                ret.attack = atk.clear_4 + ret.b2b;
                ret.b2b = true;
                like += p[CLEAR_4];
                ret.attack += atk.get_combo(++ret.combo);
                break;
            }
            if (ret.board.y_max == 0)
            {
                ret.attack = atk.pc;
                like += 99999;
            }
            ret.attack *= atk.multiplier;
            ret.send_attack = ret.attack;
            ret.under_attack.cancel(ret.send_attack);
            like -= p[PENDING_HOLD] * ret.under_attack.total() * depth;
            if (ret.attack != ret.send_attack)
            {
                like += (ret.attack - ret.send_attack) * p[CANCEL];
            }
            if (!ret.next.next.empty())
            {
                const uint32_t *mino = cache_get(ret.next.peek(), DEFAULT_R, DEFAULT_X);
                if (!ret.board.integrate(mino, DEFAULT_Y))
                {
                    ret.dead = true;
                }
            }
            ret.rating += (0. + like + p[ATTACK] * ret.attack + p[B2B] * ret.b2b + p[COMBO] * (ret.combo + atk.get_combo(ret.combo)) - 999999 * ret.dead);
        }
        void evaluation_level_3(const Status &last, Status &ret, int depth)
        {
            double like = 0;
            if (!ret.clear)
            {
                ++ret.attack_since;
                ret.cumulative_attack = 0;
            }
            else
            {
                ret.cumulative_attack += ret.send_attack;
                ret.attack_since = 0;
            }
            if (ret.attack_since < last.attack_since)
            {
                like += p[BUILD_ATTACK] * (last.attack_since - ret.attack_since) * ret.send_attack;
            }
            int val = 0;
            find_every_spin(ret.board, val);
            ret.rating += 0. + like + p[SPIKE] * (ret.cumulative_attack * ret.send_attack) + p[ASPIN_SLOT] * val;
        }
        std::vector<std::function<void(const Status &, Status &, int)>> evaluations;
        Evaluation()
        {
            evaluations.push_back([this](const Status &a, Status &b, int c)
                                  { return evaluation_level_1(a, b, c); });
            evaluations.push_back([this](const Status &a, Status &b, int c)
                                  { return evaluation_level_2(a, b, c); });
            evaluations.push_back([this](const Status &a, Status &b, int c)
                                  { return evaluation_level_3(a, b, c); });
        }
    };
}