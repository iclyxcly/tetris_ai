#pragma once
#include "board.hpp"
#include "next.hpp"
#include "mino.hpp"
#include "pending.hpp"
#include "minotemplate.h"
#include "movegen.hpp"
#include "eval.hpp"

namespace moenew
{
    void cycle(Evaluation::Status &status, MoveData &data, Evaluation::AttackTable &atk)
    {
        auto active = status.next.pop();
        const uint32_t *mino = cache_get(active, data.get_r(), data.get_x());
        status.allspin = MoveGen::immobile_global(data, active, status.board);
        status.board.paste(mino, data.get_y());
        status.clear = status.board.flush();
        status.attack = 0;

        switch (status.clear)
        {
        case 0:
            status.combo = 0;
            status.under_attack.accept(status.board, atk.messiness);
            status.under_attack.decay();
            break;
        case 1:
            if (status.allspin)
            {
                status.attack = atk.aspin_1 + status.b2b * atk.b2b;
                status.b2b = true;
            }
            else
            {
                status.attack = atk.clear_1;
                status.b2b = false;
            }
            status.attack += atk.get_combo(++status.combo);
            break;
        case 2:
            if (status.allspin)
            {
                status.attack = atk.aspin_2 + status.b2b * atk.b2b;
                status.b2b = true;
            }
            else
            {
                status.attack = atk.clear_2;
                status.b2b = false;
            }
            status.attack += atk.get_combo(++status.combo);
            break;
        case 3:
            if (status.allspin)
            {
                status.attack = atk.aspin_3 + status.b2b * atk.b2b;
                status.b2b = true;
            }
            else
            {
                status.attack = atk.clear_3;
                status.b2b = false;
            }
            status.attack += atk.get_combo(++status.combo);
            break;
        case 4:
            status.attack = atk.clear_4 + status.b2b * atk.b2b;
            status.b2b = true;
            status.attack += atk.get_combo(++status.combo);
            break;
        }
        if (status.board.y_max == 0)
        {
            status.attack = atk.pc;
        }
        status.send_attack = status.attack;
        if (!status.attack)
        {
            ++status.attack_since;
            status.cumulative_attack = 0;
        }
        else
        {
            status.attack_since = 0;
            status.cumulative_attack += status.attack;
        }
        status.under_attack.cancel(status.send_attack);
        const uint32_t *next = cache_get(status.next.peek(), DEFAULT_R, DEFAULT_X);
        if (!status.board.integrate(next, DEFAULT_Y))
        {
            status.dead = true;
        }
    }
}