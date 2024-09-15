#include "node.hpp"
#include "movegen.hpp"
#include "eval.hpp"
#include "minotemplate.h"

namespace moenew
{
    class Engine
    {
    private:
        using nodeset = std::pair<Evaluation::Status, MoveData>;
        Evaluation eval_engine;
        NodeManager beam;
        bool can_hold;

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
        void submit_form(MoveData &data, Evaluation::Status &status, bool can_hold)
        {
            this->can_hold = can_hold;
            beam.create_root(data, status, eval_engine.p.param[Evaluation::RATIO]);
            expand_first();
        }
        void expand_first()
        {
            auto &queue = beam.get_task();
            auto *data = queue.front().get();
            queue.pop();
            auto next = data->status.next;
            std::vector<nodeset> set;
            {
                MoveGen search(data->status.board, data->decision, next.peek());
                search.start();
                for (const auto &landpoint : search.result)
                {
                    auto new_stat = data->status;
                    auto x = landpoint.get_x();
                    auto y = landpoint.get_y();
                    auto r = landpoint.get_r();
                    new_stat.board.paste(cache_get(search.type, r, x), y);
                    new_stat.allspin = search.immobile(landpoint);
                    new_stat.clear = new_stat.board.flush();
                    set.emplace_back(std::make_pair(new_stat, landpoint));
                }
            }
            if (can_hold && next.swap())
            {
                {
                    MoveGen search(data->status.board, data->decision, next.peek());
                    search.start();
                    for (const auto &landpoint : search.result)
                    {
                        auto new_stat = data->status;
                        auto x = landpoint.get_x();
                        auto y = landpoint.get_y();
                        auto r = landpoint.get_r();
                        new_stat.board.paste(cache_get(search.type, r, x), y);
                        new_stat.allspin = search.immobile(landpoint);
                        new_stat.clear = new_stat.board.flush();
                        set.emplace_back(std::make_pair(new_stat, landpoint));
                    }
                }
            }
            for (auto &func : eval_engine.evaluations)
            {
                for (auto &new_data : set)
                {
                    func(data->status, new_data.first);
                }
            }
        }
    };
}