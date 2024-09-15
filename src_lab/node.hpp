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
        std::vector<Node *> children;
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
        std::priority_queue<Node *, std::vector<Node *>, NodeSort> row_result;
        std::atomic<double> min, max;
        double ratio;
        std::queue<Node *> row_task;
        std::queue<Node *> task;
        NodeResult stable, beta;
        void insert_child(const Evaluation::Status &result, const Node *parent, const MoveData &move)
        {
            Node *child = new Node;
            child->acc = parent->acc + result.rating;
            child->decision = move;
            child->parent = parent;
            child->status = result;
            child->version = parent->version + 1;
            row_result.push(child);
            min = std::min<double>(child->acc, min);
            max = std::max<double>(child->acc, max);
        }

    public:
        NodeManager(MoveData &loc, Evaluation::Status &status)
        {
            root = new Node;
            root->acc = 0;
            root->decision = loc;
            root->parent = nullptr;
            root->status = status;
            root->version = 0;
            task.push(root);
        }
        void try_insert(const Evaluation::Status &result, const Node *parent, const MoveData &move)
        {
            if (row_result.size() < BEAM_WIDTH)
            {
                insert_child(result, parent, move);
                return;
            }
            if (parent->acc + result.rating < min)
            {
                return;
            }
            // else push while > beam width prune front
        }
    };
}