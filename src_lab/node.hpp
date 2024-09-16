#pragma once
#include "eval.hpp"
#include "movegen.hpp"
#include <vector>
#include <queue>
#include <limits>
#include <atomic>
#include <memory>
#include <cassert>

namespace moenew
{
    struct Node
    {
        int version;
        MoveData decision;
        bool change_hold : 1;
        Evaluation::Status status;
        std::shared_ptr<Node> parent;
        double acc;

        Node get() const
        {
            if (version == 1)
            {
                return *this;
            }
            else
            {
                return parent->get();
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
                return a->acc < b->acc;
            }
        };

        struct NodeResult
        {
            MoveData decision;
            bool change_hold;
            double rating = std::numeric_limits<double>::lowest();
        };

        nodeptr root;
        std::priority_queue<nodeptr, std::vector<nodeptr>, NodeCompare> row_result;
        std::atomic<double> max_acc{std::numeric_limits<double>::lowest()};
        double ratio;
        std::queue<nodeptr> row_task;
        std::queue<nodeptr> task;
        std::vector<nodeptr> storage;
        NodeResult result;

        void insert_child(const Evaluation::Status &status, const nodeptr &parent, const MoveData &move, bool &hold)
        {
            auto child = std::make_shared<Node>();
            child->acc = parent->acc + status.rating;
            child->decision = move;
            child->change_hold = hold;
            child->parent = parent;
            child->status = status;
            child->version = parent->version + 1;
            row_result.push(child);
            max_acc = std::max(max_acc.load(), child->acc);
        }

        void trim()
        {
            while (row_result.size() > BEAM_WIDTH)
            {
                row_result.pop();
            }
        }

    public:
        NodeManager() {};

        NodeManager(const MoveData &loc, const Evaluation::Status &status, double ratio)
            : ratio(ratio)
        {
            create_root(loc, status, ratio);
        }

        void create_root(const MoveData &loc, const Evaluation::Status &status, double ratio)
        {
            root = std::make_shared<Node>();
            root->acc = 0;
            root->decision = loc;
            root->parent = nullptr;
            root->status = status;
            root->version = 0;
            task.push(root);
        }

        void try_insert(const Evaluation::Status &status, const nodeptr &parent, const MoveData &move, bool &hold)
        {
            if (row_result.size() < BEAM_WIDTH)
            {
                insert_child(status, parent, move, hold);
                return;
            }
            
            double parentAcc = parent->acc;
            double rowResultAcc = row_result.top()->acc;

            if (((parentAcc + status.rating - rowResultAcc) / (max_acc - rowResultAcc)) < ratio)
            {
                return;
            }

            insert_child(status, parent, move, hold);
            trim();
        }

        NodeResult finalize()
        {
            trim();
            while (!row_result.empty())
            {
                task.push(row_result.top());
                storage.push_back(row_result.top());
                if (row_result.size() == 1)
                {
                    auto data = row_result.top()->get();
                    result.decision = data.decision;
                    result.change_hold = data.change_hold;
                    result.rating = data.acc;
                }
                row_result.pop();
            }
            return result;
        }

        bool prepare()
        {
            row_task = std::move(task);
            task = std::queue<nodeptr>();
            max_acc = std::numeric_limits<double>::lowest();
            return !row_task.empty();
        }

        auto &get_task()
        {
            return row_task;
        }

        void reset()
        {
            root.reset();
            row_result = std::priority_queue<nodeptr, std::vector<nodeptr>, NodeCompare>();
            max_acc = std::numeric_limits<double>::lowest();
            row_task = std::queue<nodeptr>();
            task = std::queue<nodeptr>();
            storage.clear();
            result = NodeResult();
        }
    };
}