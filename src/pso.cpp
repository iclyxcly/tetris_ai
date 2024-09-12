#include "tetris_core.h"
#include <random>
#include <stdio.h>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <atomic>

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
    TetrisParam x_max, x_min, v_max;
    PSOConfig()
    {
        w = 0.9;
        dest_w = 0.4;
        c1 = 2;
        c2 = 1;
        TetrisParam p;
        auto pso_config = [&](int index, double value, double offset, double velocity)
        {
            x_max.weight[index] = value + offset;
            x_min.weight[index] = value - offset;
            v_max.weight[index] = velocity;
        };
        auto &x = p.weight;
        pso_config(ALLSPIN_1, x[ALLSPIN_1], 100, 5);
        pso_config(ALLSPIN_2, x[ALLSPIN_2], 100, 5);
        pso_config(ALLSPIN_3, x[ALLSPIN_3], 100, 5);
        pso_config(ALLSPIN_SLOT, x[ALLSPIN_SLOT], 100, 5);
        pso_config(COMBO, x[COMBO], 1000, 10);
        pso_config(ATTACK, x[ATTACK], 1000, 10);
        pso_config(CLEAR_1, x[CLEAR_1], 100, 5);
        pso_config(CLEAR_2, x[CLEAR_2], 100, 5);
        pso_config(CLEAR_3, x[CLEAR_3], 100, 5);
        pso_config(CLEAR_4, x[CLEAR_4], 100, 5);
        pso_config(B2B, x[B2B], 100, 5);
        pso_config(ROOF, x[ROOF], 1000, 10);
        pso_config(COL_TRANS, x[COL_TRANS], 1000, 10);
        pso_config(ROW_TRANS, x[ROW_TRANS], 1000, 10);
        pso_config(HOLE_COUNT, x[HOLE_COUNT], 1000, 10);
        pso_config(HOLE_LINE, x[HOLE_LINE], 1000, 10);
        pso_config(WIDE_2, x[WIDE_2], 100, 2);
        pso_config(WIDE_3, x[WIDE_3], 100, 2);
        pso_config(WIDE_4, x[WIDE_4], 100, 2);
        pso_config(HIGH_WIDING, x[HIGH_WIDING], 100, 2);
        pso_config(AGGREGATE_HEIGHT, x[AGGREGATE_HEIGHT], 100, 2);
        pso_config(BUMPINESS, x[BUMPINESS], 100, 2);
        pso_config(HOLD_I, x[HOLD_I], 10, 0.5);
        pso_config(HOLD_SZO, x[HOLD_SZO], 10, 0.5);
        pso_config(HOLD_LJT, x[HOLD_LJT], 10, 0.5);
        pso_config(WASTE_I, x[WASTE_I], 10, 0.5);
        pso_config(WASTE_SZO, x[WASTE_SZO], 10, 0.5);
        pso_config(WASTE_LJT, x[WASTE_LJT], 10, 0.5);
        pso_config(TANK, x[TANK], 100, 5);
        pso_config(MID_GROUND, x[MID_GROUND], 1, 0.2);
        pso_config(HIGH_GROUND, x[HIGH_GROUND], 1, 0.2);
        pso_config(SEND, x[SEND], 1000, 10);
        pso_config(CANCEL, x[CANCEL], 100, 5);
        pso_config(SKIM, x[SKIM], 100, 5);
        pso_config(APL, x[APL], 100, 5);
    }
};
constexpr int WIN_REQUIREMENT = 15;
struct TetrisPlayer
{
    TetrisConfig &config;
    TetrisNextManager next;
    TetrisParam param;
    TetrisStatus status;
    TetrisMap map;
    std::string last_path;
    int count;
    int clear;
    int attack;
    int receive;
    std::size_t last_nodes;

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
        if (lines == 0)
        {
            return;
        }
        status.garbage.push_lines(lines, 1);
        receive += lines;
    }

    TetrisPlayer(TetrisConfig &config, TetrisParam &param, std::mt19937 &gen)
        : config(config), next(config), param(param), map(10, 40), count(0), clear(0), attack(0), receive(0)
    {
        status.init();
        status.garbage = TetrisPendingLineManager(gen);
    }

    bool run()
    {
        if (next.size() < 14)
        {
            push_next();
        }
        next.next();
        status.next = next;
        TetrisTree tree(map, status, config, param);
        last_path = tree.run();
        TetrisGameEmulation emu;
        emu.run(map, next, status, last_path, 1);
        ++count;
        attack += status.attack;
        clear += status.clear;
        last_nodes = tree.total_nodes;
        return !status.dead;
    }
};
enum PSOPositionType
{
    PSO_CURRENT,
    PSO_BEST_PERSONAL,
    PSO_VELOCITY,
    PSO_BEST_GLOBAL
};
std::default_random_engine engine(std::random_device{}());
std::uniform_real_distribution<double> dist(0.0, 1.0);
std::uniform_real_distribution<double> zdist(-1.0, 1.0);
struct PSOParticleData
{
    int id;
    TetrisParam pos[4];
    PSOConfig cfg;
    int generation;
    double highscore;
    bool ingame;
    void inform_global_best(const TetrisParam &global_best)
    {
        pos[PSO_BEST_GLOBAL] = global_best;
        if (id == 0)
        {
            pos[PSO_CURRENT] = global_best;
            pos[PSO_BEST_PERSONAL] = global_best;
        }
    }
    void push_new_standard(const TetrisParam &best)
    {
        if (id != 0)
        {
            return;
        }
        pos[PSO_BEST_PERSONAL] = best;
        pos[PSO_CURRENT] = best;
        ++generation;
        calc_init();
    }
    void calc_init()
    {
        if (id == 0)
        {
            return;
        }
        double r1 = dist(engine);
        double r2 = dist(engine);

        for (int i = 0; i < END_OF_PARAM; ++i)
        {
            pos[PSO_VELOCITY].weight[i] = (cfg.w * pos[PSO_VELOCITY].weight[i]) + (cfg.c1 * r1 * (pos[PSO_BEST_PERSONAL].weight[i] - pos[PSO_CURRENT].weight[i])) + (cfg.c2 * r2 * (pos[PSO_BEST_GLOBAL].weight[i] - pos[PSO_CURRENT].weight[i]));
            pos[PSO_VELOCITY].weight[i] = std::max(std::min(pos[PSO_VELOCITY].weight[i], cfg.v_max.weight[i]), -cfg.v_max.weight[i]);
            pos[PSO_CURRENT].weight[i] += pos[PSO_VELOCITY].weight[i];
            pos[PSO_CURRENT].weight[i] = std::max(std::min(pos[PSO_CURRENT].weight[i], cfg.x_max.weight[i]), cfg.x_min.weight[i]);
        }
        if (cfg.w > cfg.dest_w)
        {
            cfg.w -= 0.005;
        }
    }
    void inform_complete(const double &result)
    {
        ingame = false;
        ++generation;
        if (result >= highscore)
        {
            highscore = result;
            pos[PSO_BEST_PERSONAL] = pos[PSO_CURRENT];
        }
        else
        {
            highscore *= 0.9;
        }
        calc_init();
    }
    PSOParticleData(int id)
        : id(id), generation(0), ingame(false)
    {
        if (id == 0)
        {
            return;
        }
        for (int i = 0; i < END_OF_PARAM; ++i)
        {
            pos[PSO_VELOCITY].weight[i] = std::uniform_real_distribution<double>(-cfg.v_max.weight[i], cfg.v_max.weight[i])(engine);
            pos[PSO_CURRENT].weight[i] = pos[PSO_CURRENT].weight[i] + pos[PSO_VELOCITY].weight[i];
        }
    }
    PSOParticleData() : id(0), generation(0), ingame(false)
    {
    }
};
struct PSOSwarmManager
{
    std::vector<PSOParticleData *> swarm;
    TetrisParam global_best;

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

    void export_id(int id)
    {
        PSOParticleData *particle = nullptr;
        for (auto &p : swarm)
        {
            if (p->id == id)
            {
                particle = p;
                break;
            }
        }
        if (particle == nullptr)
        {
            return;
        }
        FILE *file = fopen("export_param.txt", "w");
        if (file == nullptr)
        {
            return;
        }
        for (int i = 0; i < END_OF_PARAM; ++i)
        {
            fprintf(file, "%lf\n", particle->pos[PSO_CURRENT].weight[i]);
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
            PSOParticleData particle_data = *particle;
            particle_data.ingame = false;
            fwrite(&particle_data, sizeof(PSOParticleData), 1, file);
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
            swarm.push_back(new PSOParticleData(particle));
        }
        fclose(file);
        return true;
    }

    void init_pso(int count)
    {
        for (int i = 0; i < count; ++i)
        {
            PSOParticleData particle(generate_new_id());
            particle.calc_init();
            swarm.push_back(new PSOParticleData(particle));
        }
        update_best();
    }

    bool matchcmp(PSOParticleData *a, PSOParticleData *b)
    {
        return a->generation < b->generation;
    }

    std::pair<PSOParticleData *, PSOParticleData *> find_match_pair()
    {
        PSOParticleData *a = nullptr;
        PSOParticleData *b = nullptr;

        for (int i = swarm.size() - 1; i >= 0; --i)
        {
            if ((b == nullptr && swarm[i]->id != 0 && !swarm[i]->ingame) || (b != nullptr && swarm[i]->id != 0 && !swarm[i]->ingame && matchcmp(swarm[i], b)))
            {
                b = swarm[i];
            }
            if (swarm[i]->id == 0)
            {
                a = swarm[i];
            }
        }
        if (a != nullptr)
        {
            a->ingame = true;
        }
        if (b != nullptr)
        {
            b->ingame = true;
        }
        return std::make_pair(a, b);
    }

    int generate_new_id()
    {
        int id = 0;
        for (std::size_t i = 0; i < swarm.size(); ++i)
        {
            if (id <= swarm[i]->id)
            {
                id = swarm[i]->id + 1;
            }
        }
        return id;
    }

    PSOParticleData get_best()
    {
        PSOParticleData best;
        for (auto &particle : swarm)
        {
            if (particle->id == 0)
            {
                best = *particle;
                break;
            }
        }
        return best;
    }

    void update_best()
    {
        auto best = get_best();
        if (global_best != best.pos[PSO_BEST_PERSONAL])
        {
            global_best = best.pos[PSO_BEST_PERSONAL];
            for (auto &particle : swarm)
            {
                particle->inform_global_best(global_best);
            }
            export_best(global_best);
            export_data();
        }
    }

    void inform_complete(PSOParticleData *a, PSOParticleData *b, const int &a_gen, bool b_win, double b_score)
    {
        bool a_cancelled = a->generation != a_gen;
        if (!a_cancelled && b_win)
        {
            a->push_new_standard(b->pos[PSO_CURRENT]);
            update_best();
        }
        b->inform_complete(b_score);
    }
};

int main(void)
{
    const std::size_t thread_count = (std::size_t)(std::thread::hardware_concurrency() - 1);
    {
        TetrisMinoManager mino("botris_srs.json");
    }
    srand(time(nullptr));
    PSOSwarmManager s_mgr;
    if (!s_mgr.import_data())
    {
        s_mgr.init_pso(thread_count + 1);
    }
    std::recursive_mutex mtx;
    std::vector<std::thread> threads;
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
    config.target_time = 100;

    std::atomic<std::size_t> view_index{0};
    std::atomic<bool> view{false};
    std::atomic<bool> force_win{false};

    for (std::size_t i = 0; i < thread_count; ++i)
    {
        threads.emplace_back(std::thread([&, i]()
                                         {
            std::size_t index = i + 1;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dis(0, 9);
            std::uniform_int_distribution<int> mess_dis(0, 99);
            while (true)
            {
                mtx.lock();
                auto match_result = s_mgr.find_match_pair();
                if (match_result.first == nullptr || match_result.second == nullptr)
                {
                    mtx.unlock();
                    continue;
                }
                int f_generation = match_result.first->generation;
                mtx.unlock();
                int win[2] = {0, 0};
                auto view_func = [&](TetrisPlayer &player_1, TetrisPlayer &player_2)
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
                    std::string nexts_1 = player_1.next.to_string();
                    std::string nexts_2 = player_2.next.to_string();
                    nexts_1.resize(6);
                    nexts_2.resize(6);
                    uint16_t up_1 = player_1.status.garbage.total_damage();
                    uint16_t up_2 = player_2.status.garbage.total_damage();
                    snprintf(out, sizeof out, "HOLD = %c NEXT = %s UP = %d COMBO = %d B2B = %d, APP = %3.2f, ID = %d, WIN = %d, NODE = %d, PATH = %s\n"
                                              "HOLD = %c NEXT = %s UP = %d COMBO = %d B2B = %d, APP = %3.2f, ID = %d, WIN = %d, NODE = %d, PATH = %s\n",
                             mino_to_char[player_1.next.hold], nexts_1.c_str(), up_1, player_1.status.combo, player_1.status.b2b, (double)player_1.attack / (double)player_1.count, match_result.first->id, win[0], player_1.last_nodes, player_1.last_path.c_str(),
                             mino_to_char[player_2.next.hold], nexts_2.c_str(), up_2, player_2.status.combo, player_2.status.b2b, (double)player_2.attack / (double)player_2.count, match_result.second->id, win[1], player_2.last_nodes, player_2.last_path.c_str());
                    TetrisMap map_copy1 = player_1.map;
                    TetrisMap map_copy2 = player_2.map;
                    {
                        TetrisActive next_mino(config.default_x, config.default_y, config.default_r, player_1.next.queue.front());
                        TetrisInstructor temporal_instruct(map_copy1, player_1.next.queue.front());
                        temporal_instruct.dropless_attach(map_copy1, next_mino);
                    }
                    {
                        TetrisActive next_mino(config.default_x, config.default_y, config.default_r, player_2.next.queue.front());
                        TetrisInstructor temporal_instruct(map_copy2, player_2.next.queue.front());
                        temporal_instruct.dropless_attach(map_copy2, next_mino);
                    }
                    for (int y = config.default_y + 4; y >= 0; --y)
                    {
                        snprintf(out + strlen(out), sizeof out - strlen(out), "%2d|", y);
                        for (int x = 0; x < 10; ++x)
                        {
                            strcat(out, map_copy1.full(x, y) ? box_1 : box_0);
                        }
                        snprintf(out + strlen(out), sizeof out - strlen(out), "|  %2d|", y);
                        for (int x = 0; x < 10; ++x)
                        {
                            strcat(out, map_copy2.full(x, y) ? box_1 : box_0);
                        }
                        strcat(out, "|\r\n");
                    }
                    printf("%s", out);
                    //usleep(1000000);
                };
                static int max_count = 1000;
                double b_stats = 0;
                while (win[0] < WIN_REQUIREMENT && win[1] < WIN_REQUIREMENT && (std::abs(win[0] - win[1]) < 5 || win[1] > win[0]))
                {
                    TetrisPlayer player_1(config, match_result.first->pos[PSO_CURRENT], gen);
                    TetrisPlayer player_2(config, match_result.second->pos[PSO_CURRENT], gen);
                    while (player_1.run() && player_2.run() && player_1.count < max_count)
                    {
                        if (player_1.status.send_attack > player_2.status.send_attack)
                        {
                            player_2.push_damage(player_1.status.send_attack - player_2.status.send_attack);
                        }
                        else if (player_2.status.send_attack > player_1.status.send_attack)
                        {
                            player_1.push_damage(player_2.status.send_attack - player_1.status.send_attack);
                        }
                        view_func(player_1, player_2);
                        if (f_generation != match_result.first->generation)
                        {
                            break;
                        }
                        if (view_index == index && force_win)
                        {
                            win[1] = WIN_REQUIREMENT;
                            break;
                        }
                    }
                    b_stats += ((double)player_2.attack / (double)player_2.clear) +
                                ((double)player_2.attack / (double)player_2.count) +
                                ((double)player_2.clear / 10000.0) +
                                ((double)player_2.count / 100000.0);
                    if (f_generation != match_result.first->generation)
                    {
                        break;
                    }
                    if (player_1.status.dead)
                    {
                        ++win[1];
                    }
                    else if (player_2.status.dead)
                    {
                        ++win[0];
                    }
                    else
                    {
                        double app_1 = (double)player_1.attack / (double)player_1.clear;
                        double app_2 = (double)player_2.attack / (double)player_2.clear;
                        if (app_1 > app_2)
                        {
                            ++win[0];
                        }
                        else if (app_2 > app_1)
                        {
                            ++win[1];
                        }
                        else
                        {
                            ++win[0];
                            ++win[1];
                        }
                    }
                }
                mtx.lock();
                s_mgr.inform_complete(match_result.first, match_result.second, f_generation, win[1] > win[0], win[1] + b_stats);
                mtx.unlock();
                if (view_index == index && force_win)
                {
                    force_win = false;
                }
            } }));
    }
    while (true)
    {
        char input[256];
        std::cin.getline(input, 255);
        if (input[0] == 'v')
        {
            view = !view;
            if (!view)
            {
                view_index = 0;
            }
        }
        if (input[0] == 'q')
        {
            break;
        }
        if (input[0] == 's')
        {
            mtx.lock();
            for (auto &particle : s_mgr.swarm)
            {
                printf("ID = %4d, HIGH = %8.4f, GEN = %4d, INGAME = %d\n", particle->id, particle->highscore, particle->generation, particle->ingame);
            }
            mtx.unlock();
        }
        if (input[0] == 'e')
        {
            int id = atoi(input + 1);
            printf("EXPORTING ID = %d\n", id);
            mtx.lock();
            if ((std::size_t)id > s_mgr.swarm.size())
            {
                continue;
            }
            s_mgr.export_id(id);
            mtx.unlock();
        }
        if (input[0] == '+')
        {
            if (view_index != 0)
            {
                force_win = true;
            }
        }
    }
    s_mgr.export_data();
}