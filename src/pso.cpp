#include "tetris_core.h"
#include <random>
#include <stdio.h>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <atomic>
#include <unistd.h>

using namespace TetrisAI;

std::map<uint8_t, char> mino_to_char = {
    {S, 'S'},
    {Z, 'Z'},
    {L, 'L'},
    {J, 'J'},
    {T, 'T'},
    {O, 'O'},
    {I, 'I'},
    {EMPTY, ' '}};

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
constexpr int PSO_PARTICLE_COUNT = 32;
constexpr int MAX_MATCHES = 16;
struct TetrisPlayer
{
    TetrisConfig &config;
    TetrisNextManager next;
    TetrisParam param;
    TetrisPendingLineManager pending;
    TetrisMap map;
    std::string last_path;
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
        pending.fight_lines(cur_atk);
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
        next.next();
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
        last_path = result.front().path;
        last_path += "V";
        for (auto &path : last_path)
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
                attack += atk.ass + b2b;
                cur_atk += atk.ass + b2b;
                b2b = 1;
            }
            else
            {
                b2b = 0;
            }
            attack += atk.combo_table[++combo];
            cur_atk += atk.combo_table[++combo];
            break;
        case 2:
            if (spin_type == 3)
            {
                attack += atk.asd + b2b;
                cur_atk += atk.asd + b2b;
                b2b = 1;
            }
            else
            {
                b2b = 0;
                attack += 1;
                cur_atk += 1;
            }
            attack += atk.combo_table[++combo];
            cur_atk += atk.combo_table[++combo];
            break;
        case 3:
            if (spin_type == 3)
            {
                attack += atk.ast + b2b;
                cur_atk += atk.ast + b2b;
                b2b = 1;
            }
            else
            {
                b2b = 0;
                attack += 2;
                cur_atk += 2;
            }
            attack += atk.combo_table[++combo];
            cur_atk += atk.combo_table[++combo];
            break;
        case 4:
            attack += 4 + b2b;
            cur_atk += 4 + b2b;
            b2b = 1;
            attack += atk.combo_table[++combo];
            cur_atk += atk.combo_table[++combo];
            break;
        }
        ++count;
        pending.fight_lines(cur_atk);
        TetrisActive next_mino(config.default_x, config.default_y, config.default_r, next.queue.front());
        dead = instructor.check_death(map, next_mino);
        return !dead;
    }
};
enum PSOPositionType
{
    PSO_CURRENT,
    PSO_BEST_PERSONAL,
    PSO_VELOCITY,
    PSO_BEST_GLOBAL
};
struct PSOParticleData
{
    int id;
    TetrisParam pos[4];
    PSOConfig config;
    double best_score;
    double score;
    int generation;
    int matches;
    void report_score_type_1(double value)
    {
        double new_score = score * matches;
        ++matches;
        new_score += value;
        score = new_score / matches;
    }
    void inform_global_best(const TetrisParam &global_best)
    {
        pos[PSO_BEST_GLOBAL] = global_best;
    }
    void calc_init()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dis(0.0, 1.0);
        std::uniform_int_distribution<int> int_dis(-10, 10);
        double r1 = dis(gen);
        double r2 = dis(gen);

        for (int i = 0; i < END_OF_PARAM; ++i)
        {
            if (pos[PSO_VELOCITY].weight[i] == 0)
            {
                pos[PSO_VELOCITY].weight[i] = int_dis(gen);
            }
            if (pos[PSO_BEST_PERSONAL].weight[i] == 0)
            {
                pos[PSO_BEST_PERSONAL].weight[i] = pos[PSO_CURRENT].weight[i];
            }
            if (pos[PSO_BEST_GLOBAL].weight[i] == 0)
            {
                pos[PSO_BEST_GLOBAL].weight[i] = pos[PSO_CURRENT].weight[i];
            }
            pos[PSO_VELOCITY].weight[i] = (config.w * pos[PSO_VELOCITY].weight[i]) + (config.c1 * r1 * (pos[PSO_BEST_PERSONAL].weight[i] - pos[PSO_CURRENT].weight[i])) + (config.c2 * r2 * (pos[PSO_BEST_GLOBAL].weight[i] - pos[PSO_CURRENT].weight[i]));
            pos[PSO_CURRENT].weight[i] += pos[PSO_VELOCITY].weight[i];
        }
        if (config.w > config.dest_w)
        {
            config.w -= 0.01;
        }
    }
    void advance()
    {
        if (score > best_score)
        {
            best_score = score;
            pos[PSO_BEST_PERSONAL] = pos[PSO_CURRENT];
        }
        ++generation;
        score = 0;
        matches = 0;
        calc_init();
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
    std::vector<int> pending_updates;
    TetrisParam global_best;
    struct PSOSwarmSorter
    {
        bool operator()(const PSOParticleData &a, const PSOParticleData &b)
        {
            return a.score > b.score;
        }
    };

    void export_best(TetrisParam &param)
    {
        FILE *file = fopen("best_param.txt", "w");
        if (file == nullptr)
        {
            return;
        }
        for (int i = 0; i < END_OF_PARAM; ++i)
        {
            fprintf(file, "%lf\n", param.weight[i]);
        }
        fclose(file);
    }

    void export_data()
    {
        FILE *file = fopen("pso_data.bin", "wb");
        if (file == nullptr)
        {
            return;
        }
        fwrite(&global_best, sizeof(TetrisParam), 1, file);
        for (auto &particle : swarm)
        {
            fwrite(&particle, sizeof(PSOParticleData), 1, file);
        }
        fclose(file);
    }

    bool import_data()
    {
        FILE *file = fopen("pso_data.bin", "rb");
        if (file == nullptr)
        {
            return false;
        }
        fread(&global_best, sizeof(TetrisParam), 1, file);
        PSOParticleData particle;
        while (fread(&particle, sizeof(PSOParticleData), 1, file) == 1)
        {
            swarm.push_back(particle);
        }
        fclose(file);
        return true;
    }

    void init_pso()
    {
        TetrisParam param;
        for (int i = 0; i < PSO_PARTICLE_COUNT; ++i)
        {
            PSOParticleData particle(generate_new_id(), param);
            particle.calc_init();
            swarm.push_back(particle);
        }
        std::sort(swarm.begin(), swarm.end(), PSOSwarmSorter());
    }

    PSOParticleData find_match()
    {
        PSOParticleData low;
        low.generation = low.matches = std::numeric_limits<int>::max();
        for (auto &particle : swarm)
        {
            if ((particle.matches < low.matches && particle.generation <= low.generation) || particle.generation < low.generation)
            {
                low = particle;
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

    void return_base(PSOParticleData &particle)
    {
        for (int i = std::max<int>(swarm.size(), borrowed_ids.size()); i >= 0; --i)
        {
            if ((std::size_t)i < swarm.size() && swarm[i].id == particle.id)
            {
                swarm.erase(swarm.begin() + i);
            }
            if ((std::size_t)i < borrowed_ids.size() && borrowed_ids[i] == particle.id)
            {
                borrowed_ids.erase(borrowed_ids.begin() + i);
            }
            if ((std::size_t)i < pending_updates.size() && pending_updates[i] == particle.id)
            {
                pending_updates.erase(pending_updates.begin() + i);
                particle.inform_global_best(global_best);
            }
        }
        swarm.push_back(particle);
        std::sort(swarm.begin(), swarm.end(), PSOSwarmSorter());
        update_best();
    }

    void insert_new(const TetrisParam &param)
    {
        PSOParticleData particle;
        particle.id = generate_new_id();
        particle.pos[PSO_BEST_PERSONAL] = param;
        particle.pos[PSO_BEST_GLOBAL] = global_best;
        particle.calc_init();
        swarm.push_back(particle);
        std::sort(swarm.begin(), swarm.end(), PSOSwarmSorter());
    }

    void insert_copy()
    {
        auto best = get_best();
        best.id = generate_new_id();
        swarm.push_back(best);
        std::sort(swarm.begin(), swarm.end(), PSOSwarmSorter());
    }

    void update_best()
    {
        auto best = get_best();
        if (global_best != best.pos[PSO_BEST_PERSONAL])
        {
            global_best = best.pos[PSO_BEST_PERSONAL];
            for (auto &particle : swarm)
            {
                particle.inform_global_best(global_best);
            }
            pending_updates = borrowed_ids;
            export_best(global_best);
        }
    }
};

int main(void)
{
    {
        TetrisMinoManager mino("botris_srs.json");
    }
    srand(static_cast<unsigned>(time(nullptr)));
    PSOSwarmManager s_mgr;
    if (!s_mgr.import_data())
    {
        s_mgr.init_pso();
    }
    const int thread_count = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    std::recursive_mutex mtx;
    TetrisConfig config;
    config.allow_D = true;
    config.allow_d = true;
    config.allow_LR = true;
    config.allow_lr = true;
    config.allow_x = false;
    config.can_hold = true;
    config.default_x = 3;
    config.default_y = 17;
    config.default_r = 0;
    config.target_time = 20;

    std::atomic<int> view_index{0};
    std::atomic<bool> view{false};

    for (int i = 0; i < thread_count; ++i)
    {
        threads.push_back(std::thread([&]()
                                      {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dis(0, 9);
            std::uniform_int_distribution<int> mess_dis(0, 99);
            std::uniform_int_distribution<int> line_dis(2, 3);
            std::uniform_int_distribution<int> piece_dis(2, 2);
            int index = i + 1;
            mtx.lock();
            mtx.unlock();
            while (true)
            {
                mtx.lock();
                PSOParticleData particle = s_mgr.find_match();
                mtx.unlock();
                TetrisPlayer player(config, particle.pos[PSO_CURRENT], dis, mess_dis, gen);
                auto view_func = [&]()
                {
                    if (view && view_index == 0)
                    {
                        mtx.lock();
                        if (view && view_index == 0)
                        {
                            view_index = index;
                        }
                        mtx.unlock();
                    }
                    if (index != view_index)
                    {
                        return;
                    }

                    char out[81920] = "";
                    char box_0[3] = "  ";
                    char box_1[3] = "[]";

                    out[0] = '\0';
                    std::deque<uint8_t> nexts;
                    std::queue<uint8_t> ori = player.next.queue;
                    for (int i = 0; i < 6; ++i)
                    {
                        nexts.push_back(ori.front());
                        ori.pop();
                    }
                    snprintf(out, sizeof out, "HOLD = %c NEXT = %c%c%c%c%c%c COMBO = %d B2B = %d IDX = %d\n",
                        mino_to_char[player.next.hold], mino_to_char[nexts[0]], mino_to_char[nexts[1]], mino_to_char[nexts[2]], mino_to_char[nexts[3]], mino_to_char[nexts[4]], mino_to_char[nexts[5]], player.combo, player.b2b, particle.id);
                    TetrisMap map_copy1 = player.map;
                    TetrisActive next_mino(config.default_x, config.default_y, config.default_r, player.next.queue.front());
                    TetrisInstructor temporal_instruct(map_copy1, player.next.queue.front());
                    temporal_instruct.dropless_attach(map_copy1, next_mino);
                    for (int y = config.default_y + 4; y >= 0; --y)
                    {
                        sprintf(out + strlen(out), "%2d|", y);
                        for (int x = 0; x < 10; ++x)
                        {
                            strcat(out, map_copy1.full(x, y) ? box_1 : box_0);
                        }
                        // strcat(out, "  ");
                        // for (int x = 0; x < 10; ++x)
                        // {
                        //     strcat(out, map_copy2.full(x, y) ? box_1 : box_0);
                        // }
                        strcat(out, "|\r\n");
                    }
		            printf("%s", out);
                    usleep(50000);
                };
                while (player.run())
                {
                    int mino_gen = piece_dis(gen);
                    if (player.count % mino_gen == 0)
                    {
                        uint8_t lines = line_dis(gen);
                        player.push_damage(lines);
                    }
                    view_func();
                }
                mtx.lock();
                particle.report_score_type_1((double)player.count * (std::max<double>(1, player.attack) / std::max<double>(1, player.clear)));
                if (particle.matches >= MAX_MATCHES)
                {
                    particle.advance();
                }
                s_mgr.return_base(particle);
                mtx.unlock();
            } }));
    }
    while(true)
    {
        char input[256];
        std::cin.getline(input, 256);
        if (input[0] == 'v')
        {
            mtx.lock();
            view = !view;
            if (!view)
            {
                view_index = 0;
            }
            mtx.unlock();
        }
        if (input[0] == 'q')
        {
            break;
        }
        if (input[0] == 'r')
        {
            //rank
            mtx.lock();
            auto table = s_mgr.swarm;
            mtx.unlock();
            for (auto &particle : table)
            {
                printf("ID = %4d SCORE = %7.2f, HIGH = %7.2f, GEN = %4d, MATCH = %4d\n", particle.id, particle.score, particle.best_score, particle.generation, particle.matches);
            }
        }
    }
    s_mgr.export_data();
}