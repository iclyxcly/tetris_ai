#pragma once
#include "eval.hpp"
#include "movegen.hpp"
#include <vector>
#include <queue>
#include <limits>
#include <atomic>
#include <memory>
#include <cassert>
#include <iostream>

namespace moenew
{
    std::atomic<std::size_t> max_memory(0);
    template <typename T>
    class DirtyMemoryPool
    {
    private:
        std::vector<std::shared_ptr<T>> pool;
        std::atomic<std::size_t> acquire_index;
    public:
        DirtyMemoryPool()
        {
            for (std::size_t i = 0; i < max_memory; ++i)
            {
                pool.emplace_back(std::make_shared<T>());
            }
            acquire_index = 0;
        }
        ~DirtyMemoryPool()
        {
            max_memory = std::max(max_memory.load(), pool.size());
        }
        void reset()
        {
            acquire_index = 0;
        }
        std::shared_ptr<T> &acquire()
        {
            if (acquire_index + 1 >= pool.size())
            {
                pool.emplace_back(std::make_shared<T>());
            }
            return pool[acquire_index++];
        }
        std::size_t memory_usage()
        {
            return pool.size() * sizeof(T);
        }
    };
    struct Decision : public MoveData
    {
        bool change_hold;
        Decision() = default;
        Decision(const MoveData &data, bool change_hold) : MoveData(data), change_hold(change_hold) {}
    };
    struct Node
    {
        int version;
        int trust;
        Decision decision;
        Evaluation::Status status;
        std::shared_ptr<Node> parent;
        double acc;

        Node get_first(int &trust)
        {
            trust += version;
            this->trust += trust;
            if (version == 1)
            {
                return *this;
            }
            else
            {
                return parent->get_first(trust);
            }
        }
    };
    using nodeptr = std::shared_ptr<Node>;

    class NodeManager
    {
    private:
        struct NodeCompare
        {
            bool operator()(const nodeptr &a, const nodeptr &b) const
            {
                return a->acc > b->acc;
            }
        };

        struct NodeResult
        {
            Decision decision;
            int trust = 0;
        };
        DirtyMemoryPool<Node> node_pool;
        std::priority_queue<nodeptr, std::vector<nodeptr>, NodeCompare> row_result;
        std::queue<nodeptr> row_task;
        std::queue<nodeptr> task;
        NodeResult result;

        void insert_child(const Evaluation::Status &status, const nodeptr &parent, const Decision &decision)
        {
            auto child = node_pool.acquire();
            child->trust = 0;
            child->acc = parent->acc + status.rating;
            child->decision = decision;
            child->parent = parent;
            child->status = status;
            child->version = parent->version + 1;
            row_result.push(child);
        }

        void trim(int version)
        {
            while (row_result.size() > std::max<std::size_t>(64, BEAM_WIDTH / (version + 1)))
            {
                row_result.pop();
            }
        }

    public:

        NodeManager() {};

        NodeManager(const Decision decision, const Evaluation::Status &status)
        {
            create_root(decision, status);
        }

        std::size_t memory_usage()
        {
            return node_pool.memory_usage();
        }

        void create_root(const Decision decision, const Evaluation::Status &status)
        {
            auto root = node_pool.acquire();
            root->trust = 0;
            root->acc = 0;
            root->decision = decision;
            root->parent = nullptr;
            root->status = status;
            root->version = 0;
            task.push(root);
        }

        void try_insert(const Evaluation::Status &status, const nodeptr &parent, const Decision &decision)
        {
            if (row_result.size() < std::max<std::size_t>(64, BEAM_WIDTH / (parent->version + 1)))
            {
                insert_child(status, parent, decision);
                return;
            }
            
            if (parent->acc + status.rating < row_result.top()->acc)
            {
                return;
            }

            insert_child(status, parent, decision);
            trim(parent->version);
        }

        void finalize()
        {
            while (!row_result.empty())
            {
                task.push(row_result.top());
                if (row_result.size() == 1)
                {
                    int trust = 0;
                    auto data = row_result.top()->get_first(trust);
                    if (trust > result.trust)
                    {
                        result.decision = data.decision;
                        result.trust = trust;
                    }
                }
                row_result.pop();
            }
        }

        NodeResult get_result()
        {
            node_pool.reset();
            return result;
        }

        void prepare()
        {
            row_task = std::move(task);
            task = std::queue<nodeptr>();
        }

        auto &get_task()
        {
            return row_task;
        }

        bool check_task()
        {
            return !task.empty();
        }

        void reset()
        {
            node_pool.reset();
            row_result = std::priority_queue<nodeptr, std::vector<nodeptr>, NodeCompare>();
            row_task = std::queue<nodeptr>();
            task = std::queue<nodeptr>();
            result = NodeResult();
        }
    };
}