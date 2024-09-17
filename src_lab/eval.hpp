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
            bool allspin : 1;
            bool dead : 1;
            bool b2b : 1;
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
            WASTE_S,
            WASTE_L,
            WASTE_Z,
            WASTE_I,
            WASTE_T,
            WASTE_O,
            WASTE_J,
            HOLD_S,
            HOLD_L,
            HOLD_Z,
            HOLD_I,
            HOLD_T,
            HOLD_O,
            HOLD_J,
            RATIO,
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
            }
        };
        Playstyle p;
        AttackTable atk;
        // Level 1: Evaluate the board
        // Level 2: Evaluate line clear, consequences of accepting garbage
        // Level 3: Evaluate allspin setups, wasted, held minos, and other misc stuff
        // Pruning is done each time the decision is evaluated
        void evaluation_level_1(const Status &last, Status &ret)
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
            //(0. - 0.1 * board.y_max - p[COL_TRANS] * e.col_trans - p[ROW_TRANS] * e.row_trans - p[HOLE_COUNT] * e.hole_count - p[HOLE_LINE]
            ret.rating = (0. - 0.5 * board.y_max - 0 * e.col_trans - 0.5 * e.row_trans - 0 * e.hole_count - 0 * e.hole_line + 1 * e.wide[2] + 1 * e.wide[3] + 1 * e.wide[4] - 999999. * ret.dead);
        }
        void evaluation_level_2(const Status &last, Status &ret)
        {
            double like = 0;
            ret.attack = 0;
            switch (ret.clear)
            {
            case 0:
                like += ret.under_attack.estimate() * p[TANK_CLEAN];
                ret.under_attack.accept(ret.board, atk.messiness);
                ret.under_attack.decay();
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
            ret.send_attack = ret.attack;
            ret.under_attack.cancel(ret.send_attack);
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
            ret.rating += (0. + like + 0 * ret.attack + p[B2B] * ret.b2b + p[COMBO] * (ret.combo + atk.get_combo(ret.combo)) - 999999. * ret.dead);
        }
        void evaluation_level_3(const Status &last, Status &ret)
        {
            double like = 0;
            if (!ret.attack)
            {
                ++ret.attack_since;
                ret.cumulative_attack = 0;
            }
            else
            {
                ret.cumulative_attack += ret.attack;
                ret.attack_since = 0;
            }
            if (ret.attack_since < last.attack_since)
            {
                like += p[BUILD_ATTACK] * (last.attack_since - ret.attack_since) * ret.attack;
            }
            ret.rating += 0. + like + 3 * (ret.cumulative_attack * ret.attack);
        }
        std::vector<std::function<void(const Status &, Status &)>> evaluations;
        Evaluation()
        {
            evaluations.push_back([this](const Status &a, Status &b)
                                  { return evaluation_level_1(a, b); });
            evaluations.push_back([this](const Status &a, Status &b)
                                  { return evaluation_level_2(a, b); });
            evaluations.push_back([this](const Status &a, Status &b)
                                  { return evaluation_level_3(a, b); });
        }
    };
}