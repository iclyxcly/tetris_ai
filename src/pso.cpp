#include "tetris_core.h"
#include <random>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>

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
constexpr int PSO_PARTICLE_COUNT = 32;
constexpr int MAX_MATCHES = 50;
struct TetrisPlayer
{
    TetrisConfig &config;
    TetrisNextManager next;
    TetrisParam param;
    TetrisPendingLineManager pending;
    TetrisMap map;
    int b2b;
    int combo;
    int count;
    int clear;
    int cur_atk;
    int attack;
    int receive;
    bool dead;

    void decay()
    {
        for (auto &line : pending.pending)
        {
            line.at_depth--;
        }
    }

    void push_next()
    {
        std::vector<uint8_t> nexts = {S, L, Z, I, T, O, J};
        std::shuffle(nexts.begin(), nexts.end(), std::mt19937(std::random_device()()));
        for (auto mino : nexts)
        {
            next.push(mino);
        }
    }

    void push_damage(uint8_t lines)
    {
        pending.push_lines(lines, 1);
        receive += lines;
    }

    TetrisPlayer(TetrisConfig &config, TetrisParam &param, std::uniform_int_distribution<int> &dis, std::uniform_int_distribution<int> &mess_dis, std::mt19937 &gen)
        : config(config), next(config), param(param), pending(dis, mess_dis, gen), map(10, 40), b2b(0), combo(0), count(0), clear(0), cur_atk(0), attack(0), receive(0), dead(false)
    {
    }

    bool run()
    {
        if (dead)
        {
            return false;
        }
        if (next.size() < 14)
        {
            push_next();
        }
        decay();
        TetrisStatus status(b2b, combo, next, pending);
        TetrisTree runner(map, config, status, param);
        auto result = runner.run();
        if (result.empty())
        {
            dead = true;
            return false;
        }
        if (result.front().path[0] == 'v')
        {
            next.change_hold();
        }
        TetrisInstructor instructor(map, next.active.type);
        int spin_type = 0;
        int current_clear = 0;
        for (auto &path : result.front().path)
        {
            switch (path)
            {
            case 'v':
                break;
            case 'V':
                instructor.build_snapshot(next.active);
                spin_type = instructor.immobile(next.active) ? 3 : 0;
                instructor.attach(map, next.active);
                clear += current_clear = map.flush();
                map.scan();
                break;
            case 'l':
                instructor.l(next.active);
                break;
            case 'r':
                instructor.r(next.active);
                break;
            case 'L':
                instructor.L(next.active);
                break;
            case 'R':
                instructor.R(next.active);
                break;
            case 'd':
                instructor.d(next.active);
                break;
            case 'D':
                instructor.D(next.active);
                break;
            case 'x':
                instructor.x(next.active);
                break;
            case 'c':
                instructor.c(next.active);
                break;
            case 'z':
                instructor.z(next.active);
                break;
            }
        }
        switch (current_clear)
        {
        case 0:
            combo = 0;
            pending.take_all_damage(map, atk.messiness, 0);
            break;
        case 1:
            if (spin_type == 3)
            {
                attack += cur_atk = atk.ass + b2b;
                b2b = 1;
            }
            else
            {
                b2b = 0;
            }
            attack += cur_atk += atk.combo_table[++combo];
            break;
        case 2:
            if (spin_type == 3)
            {
                attack += cur_atk = atk.asd + b2b;
                b2b = 1;
            }
            else
            {
                b2b = 0;
                attack += cur_atk = 1;
            }
            attack += cur_atk += atk.combo_table[++combo];
            break;
        case 3:
            if (spin_type == 3)
            {
                attack += cur_atk = atk.ast + b2b;
                b2b = 1;
            }
            else
            {
                b2b = 0;
                attack += cur_atk = 2;
            }
            attack += cur_atk += atk.combo_table[++combo];
            break;
        case 4:
            attack += cur_atk = 4 + b2b;
            b2b = 1;
            attack += cur_atk += atk.combo_table[++combo];
            break;
        }
        ++count;
        pending.fight_lines(cur_atk);
        dead = instructor.check_death(map, next.active);
        return !dead;
    }
};
struct PSOParticleData
{
    int id;
    TetrisParam pos[3];
    PSOConfig config;
    double best_score;
    double score;
    int generation;
    int matches;
    void report_score_type_1(int mino_count)
    {
        double new_score = score * matches;
        ++matches;
        new_score += mino_count;
        score = new_score / matches;
    }
    void inform_global_best(const TetrisParam &global_best)
    {
        pos[PSO_BEST_GLOBAL] = global_best;
    }
    void advance()
    {
        if (score < best_score)
        {
            best_score = score;
            pos[PSO_BEST_PERSONAL] = pos[PSO_CURRENT];
        }
        ++generation;
        score = 0;
        matches = 0;
        double r1 = static_cast<double>(rand()) / RAND_MAX;
        double r2 = static_cast<double>(rand()) / RAND_MAX;
        for (int i = 0; i < END_OF_PARAM; ++i)
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
    void calc_init()
    {
        double r1 = static_cast<double>(rand()) / RAND_MAX;
        double r2 = static_cast<double>(rand()) / RAND_MAX;
        for (int i = 0; i < END_OF_PARAM; ++i)
        {
            pos[PSO_CURRENT].weight[i] = pos[PSO_CURRENT].weight[i] * config.w +
                                         pos[PSO_BEST_PERSONAL].weight[i] * config.c1 * r1 +
                                         pos[PSO_BEST_GLOBAL].weight[i] * config.c2 * r2;
        }
    }
    PSOParticleData(int id, TetrisParam &param)
        : id(id), best_score(0), score(0), generation(0), matches(0)
    {
        pos[PSO_CURRENT] = param;
        pos[PSO_BEST_PERSONAL] = param;
        pos[PSO_BEST_GLOBAL] = param;
    }
    PSOParticleData() : id(0), best_score(0), score(0), generation(0), matches(0)
    {
    }
};
struct PSOSwarmManager
{
    std::vector<PSOParticleData> swarm;
    std::vector<int> borrowed_ids;
    struct PSOSwarmSorter
    {
        bool operator()(const PSOParticleData &a, const PSOParticleData &b)
        {
            return a.score > b.score;
        }
    };

    PSOParticleData find_match()
    {
        PSOParticleData low;
        low.generation = low.matches = std::numeric_limits<int>::max();
        for (auto &particle : swarm)
        {
            if (particle.matches < low.matches || particle.generation < low.generation)
            {
                low = particle;
            }
        }
        for (int i = swarm.size() - 1; i >= 0; --i)
        {
            if (swarm[i].id == low.id)
            {
                swarm.erase(swarm.begin() + i);
                break;
            }
        }
        borrowed_ids.push_back(low.id);
        return low;
    }

    int generate_new_id()
    {
        int id = 0;
        for (std::size_t i = 0; i < swarm.size(); ++i)
        {
            if (id <= swarm[i].id)
            {
                id = swarm[i].id + 1;
            }
        }
        for (std::size_t i = 0; i < borrowed_ids.size(); ++i)
        {
            if (id <= borrowed_ids[i])
            {
                id = borrowed_ids[i] + 1;
            }
        }
        return id;
    }

    PSOParticleData get_best()
    {
        PSOParticleData best;
        best.best_score = std::numeric_limits<double>::max();
        for (auto &particle : swarm)
        {
            if (particle.best_score < best.best_score)
            {
                best = particle;
            }
        }
        return best;
    }

    void insert(const PSOParticleData &particle)
    {
        swarm.push_back(particle);
        std::sort(swarm.begin(), swarm.end(), PSOSwarmSorter());
        for (int i = borrowed_ids.size() - 1; i >= 0; --i)
        {
            if (borrowed_ids[i] == particle.id)
            {
                borrowed_ids.erase(borrowed_ids.begin() + i);
                break;
            }
        }
    }

    void insert_new(const TetrisParam &param)
    {
        PSOParticleData particle;
        particle.id = generate_new_id();
        particle.pos[PSO_BEST_PERSONAL] = param;
        auto best = get_best();
        particle.pos[PSO_BEST_GLOBAL] = best.pos[PSO_BEST_GLOBAL];
        // todo: do pso logic to current

        swarm.push_back(particle);
        std::sort(swarm.begin(), swarm.end(), PSOSwarmSorter());
    }
};