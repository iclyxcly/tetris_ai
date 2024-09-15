#include "eval.hpp"
#include "movegen.hpp"
#include <vector>
#include <queue>
#include <limits>
#include <atomic>

namespace moenew
{
    struct Node
    {
        int version;
        MoveData decision;
        Evaluation::Status status;
        const Node *parent;
        double acc;

        MoveData get() const
        {
            if (version == 1)
            {
                return decision;
            }
            else
            {
                return parent->get();
            }
        }
    };

    class NodeManager
    {
    private:
        struct NodeSort
        {
            bool operator()(Node *a, Node *b)
            {
                return a->acc < b->acc;
            }
        };

        struct NodeResult
        {
            MoveData decision;
            double rating;

            NodeResult()
            {
                rating = std::numeric_limits<double>::lowest();
            }
        };

        Node *root;
        // Pointer transfer: task (parent) -> row_task (parent) -> row_result (child) -> task (new parent) | repeat
        std::priority_queue<Node *, std::vector<Node *>, NodeSort> row_result;
        std::atomic<double> max;
        double ratio;
        std::queue<Node *> row_task;
        std::queue<Node *> task;
        // address storage, for memory deallocation purpose
        std::vector<Node *> storage;
        NodeResult result;

        void insert_child(const Evaluation::Status &result, const Node *parent, const MoveData &move)
        {
            Node *child = new Node;
            child->acc = parent->acc + result.rating;
            child->decision = move;
            child->parent = parent;
            child->status = result;
            child->version = parent->version + 1;
            row_result.push(child);
            if (child->acc > max)
            {
                max = child->acc;
            }
        }

        void trim()
        {
            while (row_result.size() > BEAM_WIDTH)
            {
                delete row_result.top();
                row_result.pop();
            }
        }

    public:
        NodeManager(MoveData &loc, Evaluation::Status &status, double &ratio) : ratio(ratio)
        {
            root = new Node;
            root->acc = 0;
            root->decision = loc;
            root->parent = nullptr;
            root->status = status;
            root->version = 0;
            task.push(root);
        }

        ~NodeManager()
        {
            for (auto &node : storage)
            {
                delete node;
            }
        }

        void try_insert(const Evaluation::Status &result, const Node *parent, const MoveData &move)
        {
            double parentAcc = parent->acc;
            double rowResultAcc = row_result.top()->acc;

            if (((parentAcc + result.rating - rowResultAcc) / (max - rowResultAcc)) < ratio)
            {
                return;
            }

            insert_child(result, parent, move);
            trim();
        }

        NodeResult finalize()
        {
            trim();
            while (!row_result.empty())
            {
                task.push(row_result.top());
                storage.emplace_back(row_result.top());
                if (row_result.size() == 1)
                {
                    result.decision = row_result.top()->get();
                    result.rating = row_result.top()->acc;
                }
            }
            return result;
        }

        bool prepare()
        {
            row_task = task;
            task = {};
            max = std::numeric_limits<double>::lowest();
            return !row_task.empty();
        }

        auto &get_task()
        {
            return row_task;
        }
    };
}