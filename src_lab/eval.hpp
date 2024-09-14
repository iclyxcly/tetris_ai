#pragma once
#include "board.hpp"
#include "next.hpp"
#include "mino.hpp"
#include "pending.hpp"

namespace moenew
{
    class Evaluation
    {
    public:
        struct Status
        {
            double rating;
            int clear;
            int combo;
            int attack;
            int send_attack;
            int attack_since;
            bool allspin: 1;
            bool dead: 1;
            bool b2b: 1;
            Pending under_attack;
            Next next;
            Board board;
        };
    };
}