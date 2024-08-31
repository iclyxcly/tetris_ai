#include "tetris_core.h"
#include <random>
#include <fstream>
#include <vector>
#include <thread>

using namespace TetrisAI;

struct PSOConfig
{
    double w;
    double dest_w;
    double c1;
    double c2;
    PSOConfig()
    {
        w = 0.9;
        dest_w = 0.4;
        c1 = 2.0;
        c2 = 2.0;
    }
};
enum PSOPositionType
{
    PSO_CURRENT,
    PSO_BEST_PERSONAL,
    PSO_BEST_GLOBAL
};
struct PSOParticleData
{
    TetrisParam pos[3];
    PSOConfig config;
    double best_score;
    double score;
    int generation;
    int matches;
    void report_score_type_1(int mino_count)
    {
        ++matches;
        score += mino_count;
        if (score > best_score)
        {
            best_score = score;
            pos[PSO_BEST_PERSONAL] = pos[PSO_CURRENT];
        }
    }
    void inform_global_best(const TetrisParam &global_best)
    {
        pos[PSO_BEST_GLOBAL] = global_best;
    }
    void advance()
    {
        double r1 = static_cast<double>(rand()) / RAND_MAX;
        double r2 = static_cast<double>(rand()) / RAND_MAX;
        for (int i = 0; i < 7; ++i)
        {
            pos[PSO_CURRENT].weight[i] = pos[PSO_CURRENT].weight[i] * config.w +
                                         pos[PSO_BEST_PERSONAL].weight[i] * config.c1 * r1 +
                                         pos[PSO_BEST_GLOBAL].weight[i] * config.c2 * r2;
        }
        if (config.w > config.dest_w)
        {
            config.w -= 0.01;
        }
    }
};
struct TetrisPlayer
{
    std::queue<uint8_t> untrimmed_queue;
    TetrisNextManager next;
    TetrisPendingLine pending;
    int b2b;
    int combo;
    int count;
    
};