#include "node.hpp"
#include "movegen.hpp"
#include "eval.hpp"
#include "emu.hpp"
#include "minotemplate.h"
#include "thread.hpp"
#include <mutex>
#include <chrono>
#include <iostream>

namespace moenew
{
    class Engine
    {
    private:
        using Nodeset = std::pair<Evaluation::Status, Decision>;
        Evaluation eval_engine;
        NodeManager beam;
        FakeNext fake_next;
        bool can_hold;
        int max_set_size{0}, total{0}, depth{0};
        std::recursive_mutex mutex;
        ThreadPool pool;
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
        void process_expansion(std::vector<Nodeset> &set, const Node *data, MoveGen &search, const Evaluation::Status &template_stat, bool is_swap)
        {
            set.reserve(set.size() + search.result.size());

            for (const auto &landpoint : search.result)
            {
                auto new_stat = template_stat;
                auto x = landpoint.get_x();
                auto y = landpoint.get_y();
                auto r = landpoint.get_r();

                new_stat.board.paste(cache_get(search.type, r, x), y);
                new_stat.allspin = search.immobile(landpoint);
                new_stat.clear = new_stat.board.flush();

                set.emplace_back(std::make_pair(std::move(new_stat), Decision(landpoint, is_swap)));
            }
        }

        void expand_node(const nodeptr &sptr, bool first)
        {
            auto *data = sptr.get();
            auto next = data->status.next;
            if (next.next.empty())
            {
                return;
            }
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
            for (auto &func : eval_engine.evaluations)
            {
                for (auto &new_data : set)
                {
                    func(data->status, new_data.first, data->version + 1);
                }
            }
            for (auto &new_data : set)
            {
                beam.try_insert(new_data.first, sptr, new_data.second);
            }
            total += set.size();
            max_set_size = std::max(max_set_size, (int)set.size());
        }

        void expand_node_threaded(const nodeptr &sptr)
        {
            auto *data = sptr.get();
            auto next = data->status.next;
            if (next.next.empty())
            {
                return;
            }
            std::vector<Nodeset> set;
            set.reserve(max_set_size);
            {
                MoveGen search(data->status.board, gen_loc(), next.peek());
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
            for (auto &func : eval_engine.evaluations)
            {
                for (auto &new_data : set)
                {
                    func(data->status, new_data.first, data->version + 1);
                }
            }
            mutex.lock();
            total += set.size();
            for (auto &new_data : set)
            {
                beam.try_insert(new_data.first, sptr, new_data.second);
            }
            max_set_size = std::max(max_set_size, (int)set.size());
            mutex.unlock();
        }

        void expand(bool first = false)
        {
            auto &queue = beam.get_task();
            while (!queue.empty())
            {
                expand_node(queue.front(), first);
                queue.pop();
            }
        }

        void expand_threaded()
        {
            static std::size_t thread_count = std::thread::hardware_concurrency();
            auto &queue = beam.get_task();

            if (queue.size() < thread_count)
            {
                expand();
                return;
            }

            std::vector<std::queue<nodeptr>> chunked_queue(thread_count);

            std::size_t chunk_size = queue.size() / thread_count;
            for (std::size_t i = 0; i < thread_count; ++i)
            {
                for (std::size_t j = 0; j < chunk_size; ++j)
                {
                    chunked_queue[i].push(queue.front());
                    queue.pop();
                }
            }

            while (!queue.empty())
            {
                chunked_queue.back().push(queue.front());
                queue.pop();
            }

            for (auto &chunk : chunked_queue)
            {
                pool.enqueue([this, chunk = std::move(chunk)]() mutable
                             {
            while (!chunk.empty()) {
                expand_node_threaded(chunk.front());
                chunk.pop();
            } });
            }
            pool.wait();
        }

    public:
        Engine() {};
        MoveData get_mino_draft()
        {
            return gen_loc();
        }
        Evaluation::Status get_board_status()
        {
            return Evaluation::Status();
        }
        Evaluation::Playstyle &get_param()
        {
            return eval_engine.p;
        }
        Evaluation::AttackTable &get_attack_table()
        {
            return eval_engine.atk;
        }
        void submit_form(MoveData data, Evaluation::Status status, bool can_hold)
        {
            status.next.fill(fake_next);
            fake_next.pop();
            total = 0;
            depth = 0;
            this->can_hold = can_hold;
            beam.reset();
            beam.create_root(Decision(data, can_hold), status);
            expand(true);
        }
        Decision start()
        {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            while (high_resolution_clock::now() - now < milliseconds(100) && beam.check_task())
            {
                beam.prepare();
                expand();
                beam.finalize();
                ++depth;
            }
            printf("Total: %d, Depth: %d\n", total, depth);
            return beam.get_result().decision;
        }
        Decision start_threaded()
        {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            while (high_resolution_clock::now() - now < milliseconds(100) && beam.check_task())
            {
                beam.prepare();
                expand_threaded();
                beam.finalize();
                ++depth;
            }
            printf("Total: %d, Depth: %d\n", total, depth);
            return beam.get_result().decision;
        }
        Decision start_noded()
        {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            while (total < 100000 && beam.check_task())
            {
                beam.prepare();
                expand();
                beam.finalize();
                ++depth;
            }
            return beam.get_result().decision;
        }

        Decision start_noded_thread()
        {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            while (total < 100000 && beam.check_task())
            {
                beam.prepare();
                expand_threaded();
                beam.finalize();
                ++depth;
            }
            return beam.get_result().decision;
        }

        Decision start_depth()
        {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            while (depth < 11 && beam.check_task())
            {
                beam.prepare();
                expand();
                beam.finalize();
                ++depth;
            }
            return beam.get_result().decision;
        }

        Decision start_depth_thread()
        {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            while (depth < 11 && beam.check_task())
            {
                beam.prepare();
                expand_threaded();
                beam.finalize();
                ++depth;
            }
            return beam.get_result().decision;
        }
    };
}