#include "node.hpp"
#include "movegen.hpp"
#include "eval.hpp"
#include "emu.hpp"
#include "minotemplate.h"
#include <mutex>
#include <chrono>
#include <thread>
#include <iostream>

namespace moenew
{
    class Engine
    {
    private:
        using Nodeset = std::pair<Evaluation::Status, Decision>;
        Evaluation eval_engine;
        NodeManager beam;
        bool can_hold;
        int total;
        int depth;
        struct RatingCompare
        {
            bool operator()(Nodeset &left, Nodeset &right)
            {
                return left.first.rating < right.first.rating;
            }
        };
        void try_early_prune(std::vector<Nodeset> &set)
        {
            if (set.empty())
            {
                return;
            }
            std::sort(set.begin(), set.end(), RatingCompare());
            double max = set.end()->first.rating;
            double min = set.begin()->first.rating;
            double range = max - min;
            while (!set.empty() && (set.front().first.rating - min) / range < eval_engine.p[Evaluation::RATIO])
            {
                set.erase(set.begin());
            }
        }
        void expand_first()
        {
            auto &queue = beam.get_task();
            auto sptr = queue.front();
            auto *data = sptr.get();
            queue.pop();
            auto next = data->status.next;
            std::vector<Nodeset> set;
            {
                MoveGen search(data->status.board, data->decision, next.peek());
                search.start();
                for (const auto &landpoint : search.result)
                {
                    auto new_stat = data->status;
                    new_stat.next.pop();
                    auto x = landpoint.get_x();
                    auto y = landpoint.get_y();
                    auto r = landpoint.get_r();
                    new_stat.board.paste(cache_get(search.type, r, x), y);
                    new_stat.allspin = search.immobile(landpoint);
                    new_stat.clear = new_stat.board.flush();
                    set.emplace_back(std::make_pair(new_stat, Decision(landpoint, false)));
                }
            }
            if (can_hold && next.swap())
            {
                {
                    Decision new_decision;
                    new_decision.set_x(DEFAULT_X);
                    new_decision.set_y(DEFAULT_Y);
                    new_decision.set_r(DEFAULT_R);
                    MoveGen search(data->status.board, new_decision, next.pop());
                    search.start();
                    for (const auto &landpoint : search.result)
                    {
                        auto new_stat = data->status;
                        new_stat.next = next;
                        auto x = landpoint.get_x();
                        auto y = landpoint.get_y();
                        auto r = landpoint.get_r();
                        new_stat.board.paste(cache_get(search.type, r, x), y);
                        new_stat.allspin = search.immobile(landpoint);
                        new_stat.clear = new_stat.board.flush();
                        set.emplace_back(std::make_pair(new_stat, Decision(landpoint, true)));
                    }
                }
            }
            total += set.size();
            for (auto &func : eval_engine.evaluations)
            {
                for (auto &new_data : set)
                {
                    func(data->status, new_data.first);
                }
                try_early_prune(set);
            }
            for (auto &new_data : set)
            {
                beam.try_insert(new_data.first, sptr, new_data.second);
            }
            beam.finalize();
            ++depth;
        }
        void expand_all()
        {
            beam.prepare();
            auto &queue = beam.get_task();
            while (!queue.empty())
            {
                std::vector<Nodeset> set;
                auto sptr = queue.front();
                auto *data = sptr.get();
                queue.pop();
                auto next = data->status.next;
                if (next.next.empty())
                {
                    continue;
                }
                {
                    Decision new_decision;
                    new_decision.set_x(DEFAULT_X);
                    new_decision.set_y(DEFAULT_Y);
                    new_decision.set_r(DEFAULT_R);
                    MoveGen search(data->status.board, new_decision, next.peek());
                    search.start();
                    for (const auto &landpoint : search.result)
                    {
                        auto new_stat = data->status;
                        new_stat.next.pop();
                        auto x = landpoint.get_x();
                        auto y = landpoint.get_y();
                        auto r = landpoint.get_r();
                        new_stat.board.paste(cache_get(search.type, r, x), y);
                        new_stat.allspin = search.immobile(landpoint);
                        new_stat.clear = new_stat.board.flush();
                        set.emplace_back(std::make_pair(new_stat, Decision(landpoint, false)));
                    }
                }
                if (can_hold && next.swap())
                {
                    {
                        Decision new_decision;
                        new_decision.set_x(DEFAULT_X);
                        new_decision.set_y(DEFAULT_Y);
                        new_decision.set_r(DEFAULT_R);
                        MoveGen search(data->status.board, new_decision, next.pop());
                        search.start();
                        for (const auto &landpoint : search.result)
                        {
                            auto new_stat = data->status;
                            new_stat.next = next;
                            auto x = landpoint.get_x();
                            auto y = landpoint.get_y();
                            auto r = landpoint.get_r();
                            new_stat.board.paste(cache_get(search.type, r, x), y);
                            new_stat.allspin = search.immobile(landpoint);
                            new_stat.clear = new_stat.board.flush();
                            set.emplace_back(std::make_pair(new_stat, Decision(landpoint, true)));
                        }
                    }
                }
                total += set.size();
                for (auto &func : eval_engine.evaluations)
                {
                    for (auto &new_data : set)
                    {
                        func(data->status, new_data.first);
                    }
                    try_early_prune(set);
                }
                for (auto &new_data : set)
                {
                    beam.try_insert(new_data.first, sptr, new_data.second);
                }
            }
            beam.finalize();
            ++depth;
        }

    public:
        Engine() {};
        MoveData get_mino_draft()
        {
            return MoveData();
        }
        Evaluation::Status get_board_status()
        {
            return Evaluation::Status();
        }
        Evaluation::Playstyle &get_param()
        {
            return eval_engine.p;
        }
        AttackTable &get_attack_table()
        {
            return eval_engine.atk;
        }
        void submit_form(MoveData &data, Evaluation::Status &status, bool can_hold)
        {
            total = 0;
            depth = 0;
            this->can_hold = can_hold;
            beam.reset();
            beam.create_root(Decision(data, can_hold), status, eval_engine.p.param[Evaluation::RATIO]);
            assert(beam.prepare());
            expand_first();
        }
        Decision start()
        {
            auto now = std::chrono::high_resolution_clock::now();
            while (depth != 10)
            {
                expand_all();
            }
            printf("Total: %d\n", total);
            return beam.finalize().decision;
        }
    };
}