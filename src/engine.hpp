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
        std::atomic<int> max_set_size{0}, total{0}, beam_total{0}, depth{0};
        std::recursive_mutex mutex;
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
            total += set.size();
            for (auto &func : eval_engine.evaluations)
            {
                for (auto &new_data : set)
                {
                    func(data->status, new_data.first);
                }
            }
            for (auto &new_data : set)
            {
                beam.try_insert(new_data.first, sptr, new_data.second);
            }
            max_set_size = std::max(max_set_size.load(), (int)set.size());
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
            total += set.size();
            for (auto &func : eval_engine.evaluations)
            {
                for (auto &new_data : set)
                {
                    func(data->status, new_data.first);
                }
            }
            mutex.lock();
            for (auto &new_data : set)
            {
                beam.try_insert(new_data.first, sptr, new_data.second);
            }
            mutex.unlock();
            max_set_size = std::max(max_set_size.load(), (int)set.size());
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
            std::vector<std::thread> threads;

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
                threads.emplace_back([this, &chunk]()
                                     {
            while (!chunk.empty())
            {
                expand_node_threaded(chunk.front());
                chunk.pop();
            } });
            }

            for (auto &thread : threads)
            {
                if (thread.joinable())
                {
                    thread.join();
                }
            }
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
        void submit_form(MoveData data, Evaluation::Status &status, bool can_hold)
        {
            beam_total = 0;
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
                beam_total += beam.get_task().size();
                expand();
                beam.finalize();
                ++depth;
            }
            printf("Total: %d, Beam Total: %d, Depth: %d, Rate: %.2f\n", total.load(), beam_total.load(), depth.load(), beam.rate);
            if (depth.load() > 12)
            {
                beam.rate += 0.1;
            }
            else if (depth.load() < 10)
            {
                beam.rate -= 0.01;
                beam.rate = std::max(beam.rate, 0.05);
            }
            return beam.get_result().decision;
        }
        Decision start_threaded()
        {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();
            while (high_resolution_clock::now() - now < milliseconds(100) && beam.check_task())
            {
                beam.prepare();
                beam_total += beam.get_task().size();
                expand_threaded();
                beam.finalize();
                ++depth;
            }
            printf("Total: %d, Beam Total: %d, Depth: %d, Rate: %.2f\n", total.load(), beam_total.load(), depth.load(), beam.rate);
            if (depth.load() > 12)
            {
                beam.rate += 0.1;
            }
            else if (depth.load() < 10)
            {
                beam.rate -= 0.01;
                beam.rate = std::max(beam.rate, 0.05);
            }
            return beam.get_result().decision;
        }
        Decision start_pso()
        {
            using namespace std::chrono;
            beam.rate = 0.8;
            auto now = high_resolution_clock::now();
            while (total.load() < 40000 && beam.check_task())
            {
                beam.prepare();
                beam_total += beam.get_task().size();
                expand();
                beam.finalize();
                ++depth;
            }
            return beam.get_result().decision;
        }
    };
}