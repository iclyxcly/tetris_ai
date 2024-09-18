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
        bool change_hold;
        Evaluation::Status status;
        std::shared_ptr<Node> parent;
        double acc;

        Node get_first(int &trust)
        {
            ++trust;
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
            bool change_hold;
            int trust = 0;
        };
        nodeptr root;
        std::priority_queue<nodeptr, std::vector<nodeptr>, NodeCompare> row_result;
        std::queue<nodeptr> row_task;
        std::queue<nodeptr> task;
        NodeResult result;

        void insert_child(const Evaluation::Status &status, const nodeptr &parent, const Decision &decision)
        {
            auto child = std::make_shared<Node>();
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
            while (row_result.size() > BEAM_WIDTH + (version * 100 * rate))
            {
                row_result.pop();
            }
        }

    public:

        double rate = 1.0;

        NodeManager() {};

        NodeManager(const Decision decision, const Evaluation::Status &status)
        {
            create_root(decision, status);
        }

        void create_root(const Decision decision, const Evaluation::Status &status)
        {
            root = std::make_shared<Node>();
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
            if (row_result.size() < BEAM_WIDTH + (parent->version * 100 * rate))
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
                        result.change_hold = data.change_hold;
                        result.trust = trust;
                    }
                }
                row_result.pop();
            }
        }

        NodeResult get_result()
        {
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
            root.reset();
            row_result = std::priority_queue<nodeptr, std::vector<nodeptr>, NodeCompare>();
            row_task = std::queue<nodeptr>();
            task = std::queue<nodeptr>();
            result = NodeResult();
        }
    };
}