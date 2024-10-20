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
            AGGREGATE_HEIGHT,
            BUMPINESS,
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
        struct ASpinValue
        {
            int s;
            int l;
            int z;
            int i;
            int t;
            int o;
            int j;
        };
        static void find_every_spin(const Board &board, ASpinValue &data)
        {
            for (int y = board.y_max - 1; y >= std::max<int>(0, board.y_max - 4); --y)
            {
                int rowm1 = board.field[y - 1];
                int row0 = board.field[y];
                int row1 = board.field[y + 1];
                int row2 = board.field[y + 2];
                int row3 = board.field[y + 3];
                int row4 = board.field[y + 4];
                int count0 = __builtin_popcount(board.field[y]);
                int count1 = __builtin_popcount(board.field[y + 1]);
                int count2 = __builtin_popcount(board.field[y + 2]);
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
                                ++data.s;
                                if (count0 == 8)
                                {
                                    data.s += 2;
                                }
                                if (count1 == 8)
                                {
                                    data.s += 2;
                                }
                            }
                        }
                        // S spin
                        if (~row2 & loc_x.of(x) && ~row1 & loc_x.of(x) && ~row1 & loc_x.of(x1) && ~row0 & loc_x.of(x1))
                        {
                            // triple
                            if (row0 & loc_x.of(x) && (row2 & loc_x.of(x1) || (row0 & loc_x.of(x2) && row3 & loc_x.of(x))))
                            {
                                ++data.s;
                                if (count0 == 9)
                                {
                                    data.s += 2;
                                }
                                if (count1 == 2)
                                {
                                    data.s += 2;
                                }
                                if (count2 == 9)
                                {
                                    data.s += 2;
                                }
                            }
                        }
                        // Z spin
                        if (~row1 & loc_x.of(x) && ~row1 & loc_x.of(x1) && ~row0 & loc_x.of(x1) && ~row0 & loc_x.of(x2))
                        {
                            // double/single
                            if ((row0 & loc_x.of(x) || (y == 0 || rowm1 & loc_x.of(x))) && (row1 & loc_x.of(x2) || (row2 & loc_x.of(x) && (x3 == board.w || row0 & loc_x.of(x3)))))
                            {
                                ++data.z;
                                if (count0 == 8)
                                {
                                    data.z += 2;
                                }
                                if (count1 == 8)
                                {
                                    data.z += 2;
                                }
                            }
                        }
                        if (~row2 & loc_x.of(x1) && ~row1 & loc_x.of(x1) && ~row1 & loc_x.of(x) && ~row0 & loc_x.of(x))
                        {
                            // triple
                            if (row0 & loc_x.of(x1) && (row2 & loc_x.of(x) || ((x != 0 && row0 & loc_x.of(xm1)) && row3 & loc_x.of(x1))))
                            {
                                ++data.z;
                                if (count0 == 9)
                                {
                                    data.z += 2;
                                }
                                if (count1 == 2)
                                {
                                    data.z += 2;
                                }
                                if (count2 == 9)
                                {
                                    data.z += 2;
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
                                ++data.l;
                                if (count0 == 7)
                                {
                                    data.l += 2;
                                }
                                if (count1 == 9)
                                {
                                    data.l += 2;
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
                                    data.l += 2;
                                }
                                if (count1 == 7)
                                {
                                    data.l += 2;
                                }
                            }
                            if (cond1)
                            {
                                ++data.l;
                            }
                            if (cond2)
                            {
                                ++data.l;
                            }
                            if (cond3)
                            {
                                ++data.l;
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
                                ++data.l;
                                if (count0 == 8)
                                {
                                    data.l += 2;
                                }
                                if (count1 == 9)
                                {
                                    data.l += 2;
                                }
                                if (count2 == 9)
                                {
                                    data.l += 2;
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
                                ++data.j;
                                if (count0 == 7)
                                {
                                    data.j += 2;
                                }
                                if (count1 == 9)
                                {
                                    data.j += 2;
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
                                    data.j += 2;
                                }
                                if (count1 == 7)
                                {
                                    data.j += 2;
                                }
                            }
                            if (cond1)
                            {
                                ++data.j;
                            }
                            if (cond2)
                            {
                                ++data.j;
                            }
                            if (cond3)
                            {
                                ++data.j;
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
                                ++data.j;
                                if (count0 == 8)
                                {
                                    data.j += 2;
                                }
                                if (count1 == 9)
                                {
                                    data.j += 2;
                                }
                                if (count2 == 9)
                                {
                                    data.j += 2;
                                }
                            }
                        }
                        // T spin
                        if (~row0 & loc_x.of(x1) && ~row1 & loc_x.of(x) && ~row1 & loc_x.of(x1) && ~row1 & loc_x.of(x2) && ~row2 & loc_x.of(x1))
                        {
                            // double
                            if (row0 & loc_x.of(x) && row0 & loc_x.of(x2) && (row2 & loc_x.of(x) || row2 & loc_x.of(x2)))
                            {
                                ++data.t;
                                if (count0 == 9)
                                {
                                    data.t += 2;
                                }
                                if (count1 == 7)
                                {
                                    data.t += 2;
                                }
                                if (count2 == 9)
                                { // imperial cross?
                                    data.t += 2;
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
                                    ++data.t;
                                    if (count0 == 9)
                                    {
                                        data.t += 2;
                                    }
                                    if (count1 == 8)
                                    {
                                        data.t += 2;
                                    }
                                    if (count2 == 9)
                                    {
                                        data.t += 2;
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
                                    data.t += 2;
                                }
                            }
                            if (cond1)
                            {
                                ++data.t;
                            }
                            if (cond2)
                            {
                                ++data.t;
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
                            ++data.i;
                            if (count0 == 6)
                            {
                                data.i += 2;
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
                            ++data.i;
                            if (count0 == 9)
                            {
                                data.i += 2;
                            }
                            if (count1 == 9)
                            {
                                data.i += 2;
                            }
                            if (count2 == 9)
                            {
                                data.i += 2;
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
                            ++data.o;
                            if (count0 == 8)
                            {
                                data.o += 2;
                            }
                            if (count1 == 8)
                            {
                                data.o += 2;
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
                int aggregate_height;
                int aggregate_height_arr[32];
                int bumpiness;
                int wide[32];
            } e;
            memset(&e, 0, sizeof(e));
            const auto &board = ret.board;
            for (int y = board.y_max; y >= 0; y--)
            {
                int wide_max = 0;
                int wide_count = 0;
                int check = e.hole_count;
                for (int x = 0; x < board.w; x++)
                {
                    bool left = x == 0 || board.get(x - 1, y);
                    bool right = x + 1 >= board.w || board.get(x + 1, y);
                    bool up = y == 0 || board.get(x, y - 1);
                    if (left && right && up && !board.get(x, y))
                    {
                        e.hole_count++;
                    }
                    if (board.get(x, y))
                    {
                        if (e.aggregate_height_arr[x] == 0)
                        {
                            e.aggregate_height_arr[x] = y + 1;
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
                    if (x + 1 != board.w)
                    {
                        e.row_trans += board.get(x, y) != board.get(x + 1, y);
                    }
                    else if (x + 1 == board.w || x == 0)
                    {
                        e.row_trans += !board.get(x, y);
                    }
                }
                if (__builtin_popcount(board.field[y]) == board.w - wide_max)
                {
                    e.wide[wide_max]++;
                }
                e.hole_line += e.hole_count != check;
                if (y - 1 >= 0)
                {
                    e.col_trans += __builtin_popcount(board.field[y] ^ board.field[y - 1]);
                }
            }
            e.col_trans += board.w - __builtin_popcount(board.field[0]);
            for (int i = 0; i < board.w; i++)
            {
                e.aggregate_height += e.aggregate_height_arr[i];
                if (i != 0)
                {
                    e.bumpiness += std::abs(e.aggregate_height_arr[i - 1] - e.aggregate_height_arr[i]);
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
            ret.rating = (0. 
                - p[HEIGHT] * board.y_max
                - p[COL_TRANS] * e.col_trans
                - p[ROW_TRANS] * e.row_trans
                - p[HOLE_COUNT] * e.hole_count
                - p[HOLE_LINE] * e.hole_line
                - p[AGGREGATE_HEIGHT] * e.aggregate_height
                - p[BUMPINESS] * e.bumpiness
                + p[WIDE_2] * e.wide[2]
                + p[WIDE_3] * e.wide[3]
                + p[WIDE_4] * e.wide[4]
                - 999999 * ret.dead
            );
        }
        void evaluation_level_2(const Status &last, Status &ret, int depth)
        {
            double like = 0;
            ret.attack = 0;
            switch (ret.clear)
            {
            case 0:
                if (!ret.under_attack.lines.empty())
                {
                    like += (ret.under_attack.estimate_mess() - 4) * p[TANK_CLEAN];
                    ret.under_attack.accept(ret.board, atk.messiness);
                    ret.under_attack.decay();
                    like += !(ret.under_attack.lines[0] & 1) ? (ret.under_attack.estimate() * (ret.under_attack.estimate() - last.combo)) * p[PENDING_LOCK] : 0;
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
                like += 99999 * (ret.combo + ret.cumulative_attack);
            }
            int attack_origin = ret.attack;
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
            int safe = ret.board.get_safe(ret.next.next);
            ret.rating += (0.
                + like * (safe + 8)
                + p[ATTACK] * ret.attack * (safe + 12)
                + p[B2B] * (ret.allspin + last.allspin + (ret.clear == 4) + (last.clear == 4) + ret.b2b) * std::max<int>(1, safe - 12)
                + p[COMBO] * (ret.combo + atk.get_combo(ret.combo)) * ((DEFAULT_Y - (ret.next.next.empty() ? 0 : down_offset[ret.next.peek()][0]) - safe))
                + (atk.get_combo(ret.combo) > 3 || (ret.combo > 5 && attack_origin > 6) ? 99999 : 0)
                - 999999 * ret.dead
            );
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
                like += p[BUILD_ATTACK] * last.attack_since * ret.send_attack;
            }
            ASpinValue data;
            memset(&data, 0, sizeof(data));
            find_every_spin(ret.board, data);
            auto expect = [&](char target) -> int
            {
                if (ret.next.hold == target)
                {
                    return 0;
                }
                for (int i = 0; i < ret.next.next.size(); i++)
                {
                    if (ret.next.next[i] == target)
                    {
                        return i;
                    }
                }
                return 21;
            };
            int safe = ret.board.get_safe(ret.next.next);
            double slot = 0;
            slot += data.s * (3.0 / (expect('S') + 1));
            slot += data.l * (2.0 / (expect('L') + 1));
            slot += data.z * (3.0 / (expect('Z') + 1));
            slot += data.i * (1.0 / (expect('I') + 1));
            slot += data.t * (2.0 / (expect('T') + 1));
            slot += data.o * (4.0 / (expect('O') + 1));
            slot += data.j * (2.0 / (expect('J') + 1));
            slot *= p[ASPIN_SLOT] * std::max<int>(1, safe - 8);
            ret.rating += (0. + like + p[SPIKE] * (ret.cumulative_attack * ret.send_attack * std::max(1, safe - 12)) + slot);
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