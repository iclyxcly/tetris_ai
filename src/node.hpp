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
        Decision decision;
        Evaluation::Status status;
        std::shared_ptr<Node> parent;
        double acc;

        Node get_first()
        {
            if (version == 1)
            {
                return *this;
            }
            else
            {
                return parent->get_first();
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
            Evaluation::Status status;
            Decision decision;
        };
        std::priority_queue<nodeptr, std::vector<nodeptr>, NodeCompare> row_result;
        std::queue<nodeptr> row_task;
        std::queue<nodeptr> task;
        NodeResult result;

        void insert_child(const Evaluation::Status &status, const nodeptr &parent, const Decision &decision)
        {
            auto child = std::make_shared<Node>();
            child->acc = parent->acc + status.rating;
            child->decision = decision;
            child->parent = parent;
            child->status = status;
            child->version = parent->version + 1;
            row_result.push(child);
        }

        void trim()
        {
            while (row_result.size() > target_beam)
            {
                row_result.pop();
            }
        }

    public:
        int target_beam = BEAM_WIDTH;

        NodeManager() {};

        NodeManager(const Decision decision, const Evaluation::Status &status)
        {
            create_root(decision, status);
        }

        void create_root(const Decision decision, const Evaluation::Status &status)
        {
            auto root = std::make_shared<Node>();
            root->acc = 0;
            root->decision = decision;
            root->parent = nullptr;
            root->status = status;
            root->version = 0;
            task.push(root);
        }

        void try_insert(const Evaluation::Status &status, const nodeptr &parent, const Decision &decision)
        {
            if (row_result.size() < target_beam)
            {
                insert_child(status, parent, decision);
                return;
            }

            if (parent->acc + status.rating < row_result.top()->acc)
            {
                return;
            }

            insert_child(status, parent, decision);
            trim();
        }

        void finalize()
        {
            while (!row_result.empty())
            {
                task.push(row_result.top());
                if (row_result.size() == 1)
                {
                    auto data = row_result.top()->get_first();
                    result.status = data.status;
                    result.decision = data.decision;
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
            row_result = std::priority_queue<nodeptr, std::vector<nodeptr>, NodeCompare>();
            row_task = std::queue<nodeptr>();
            task = std::queue<nodeptr>();
            result = NodeResult();
        }
    };
}