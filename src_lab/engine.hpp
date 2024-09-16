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
        std::atomic<int> max_set_size{0};
        struct RatingCompare
        {
            bool operator()(Nodeset &left, Nodeset &right)
            {
                return left.first.rating < right.first.rating;
            }
        };
        static MoveData gen_loc()
        {
            static MoveData data;
            if (data.data == 0)
            {
                data.set_x(DEFAULT_X);
                data.set_y(DEFAULT_Y);
                data.set_r(DEFAULT_R);
            }
            return data;
        }
        void try_early_prune(std::vector<Nodeset> &set)
        {
            std::sort(set.begin(), set.end(), RatingCompare());
            double max = set.end()->first.rating;
            double min = set.begin()->first.rating;
            double range = max - min;
            while (!set.empty() && (set.front().first.rating - min) / range < eval_engine.p[Evaluation::RATIO])
            {
                set.erase(set.begin());
            }
        }
        void process_expansion(std::vector<Nodeset> &set, const Node *data, MoveGen &search, const Evaluation::Status &template_stat, bool is_swap)
        {
            for (const auto &landpoint : search.result)
            {
                auto new_stat = template_stat;
                auto x = landpoint.get_x();
                auto y = landpoint.get_y();
                auto r = landpoint.get_r();
                new_stat.board.paste(cache_get(search.type, r, x), y);
                new_stat.allspin = search.immobile(landpoint);
                new_stat.clear = new_stat.board.flush();
                set.emplace_back(std::make_pair(new_stat, Decision(landpoint, is_swap)));
            }
        }
        void expand_node(const nodeptr &sptr, bool first)
        {
            auto *data = sptr.get();
            auto next = data->status.next;
            std::vector<Nodeset> set;
            set.reserve(max_set_size);
            {
                MoveGen search(data->status.board, first ? data->decision : gen_loc(), next.peek());
                search.start();
                auto template_stat = data->status;
                template_stat.next.pop();
                process_expansion(set, data, search, template_stat, false);
            }
            if (can_hold && next.swap())
            {
                MoveGen search(data->status.board, gen_loc(), next.pop());
                search.start();
                auto template_stat = data->status;
                template_stat.next = next;
                process_expansion(set, data, search, template_stat, true);
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
            max_set_size = std::max(max_set_size.load(), (int)set.size());
        }
        void expand(bool first = false)
        {
            beam.prepare();
            auto &queue = beam.get_task();
            while (!queue.empty())
            {
                expand_node(queue.front(), first);
                queue.pop();
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
            expand(true);
        }
        Decision start()
        {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            while (high_resolution_clock::now() - now < milliseconds(100) && beam.check_task())
            {
                expand();
            }
            printf("Total: %d\n", total);
            printf("Depth: %d\n", depth);
            return beam.finalize().decision;
        }
    };
}