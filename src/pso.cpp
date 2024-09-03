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
    TetrisParam x_max, x_min, v_max;
    PSOConfig()
    {
        w = 0.9;
        dest_w = 0.4;
        c1 = 2;
        c2 = 2;
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
        pso_config(UPSTACK, x[UPSTACK], 100, 2.5);
        pso_config(COMBO, x[COMBO], 1000, 5);
        pso_config(ATTACK, x[ATTACK], 1000, 5);
        pso_config(CLEAR_1, x[CLEAR_1], 100, 2.5);
        pso_config(CLEAR_2, x[CLEAR_2], 100, 2.5);
        pso_config(CLEAR_3, x[CLEAR_3], 100, 2.5);
        pso_config(CLEAR_4, x[CLEAR_4], 100, 2.5);
        pso_config(B2B, x[B2B], 1000, 2.5);
        pso_config(ROOF, x[ROOF], 1000, 5);
        pso_config(COL_TRANS, x[COL_TRANS], 1000, 5);
        pso_config(ROW_TRANS, x[ROW_TRANS], 1000, 5);
        pso_config(HOLE_COUNT, x[HOLE_COUNT], 1000, 5);
        pso_config(HOLE_LINE, x[HOLE_LINE], 1000, 5);
        pso_config(WIDE_2, x[WIDE_2], 100, 2.5);
        pso_config(WIDE_3, x[WIDE_3], 100, 2.5);
        pso_config(WIDE_4, x[WIDE_4], 100, 2.5);
        pso_config(HIGH_WIDING, x[HIGH_WIDING], 100, 2.5);
        pso_config(AGGREGATE_HEIGHT, x[AGGREGATE_HEIGHT], 100, 2.5);
        pso_config(BUMPINESS, x[BUMPINESS], 100, 2.5);
        pso_config(HOLD_I, x[HOLD_I], 10, 1);
        pso_config(HOLD_SZO, x[HOLD_SZO], 10, 1);
        pso_config(HOLD_LJT, x[HOLD_LJT], 10, 1);
        pso_config(WASTE_I, x[WASTE_I], 10, 1);
        pso_config(WASTE_SZO, x[WASTE_SZO], 10, 1);
        pso_config(WASTE_LJT, x[WASTE_LJT], 10, 1);
        pso_config(ATTACK_FORECAST, x[ATTACK_FORECAST], 10, 1);
        pso_config(ALLSPIN_FORECAST, x[ALLSPIN_FORECAST], 10, 1);
        pso_config(ALLSPIN_CHAIN, x[ALLSPIN_CHAIN], 100, 5);
        pso_config(TANK, x[TANK], 100, 2.5);
        pso_config(MID_GROUND, x[MID_GROUND], 1, 0.1);
        pso_config(HIGH_GROUND, x[HIGH_GROUND], 1, 0.1);
    }
};
constexpr int PSO_PARTICLE_COUNT = 32;
constexpr int WIN_REQUIREMENT = 15;
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
            pending.take_all_damage(map, atk.messiness);
            pending.decay();
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
        if (!map.roof)
        {
            attack = 10;
            cur_atk = 10;
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
            cfg.w -= 0.01;
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
        else {
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

    void init_pso()
    {
        for (int i = 0; i < PSO_PARTICLE_COUNT; ++i)
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
        }
    }

    void inform_complete(PSOParticleData *a, PSOParticleData *b, const int &a_gen, bool b_win, double b_score)
    {
        bool a_cancelled = a->generation != a_gen;
        if (!a_cancelled && !b_win)
        {
            b->inform_complete(b_score);
        }
        else if (!a_cancelled && b_win)
        {
            a->push_new_standard(b->pos[PSO_CURRENT]);
            update_best();
            b->inform_complete(b_score);
        }
        else {
            b->ingame = false;
        }
    }
};

int main(void)
{
    {
        TetrisMinoManager mino("botris_srs.json");
    }
    srand(time(NULL));
    PSOSwarmManager s_mgr;
    if (!s_mgr.import_data())
    {
        s_mgr.init_pso();
    }
    const std::size_t thread_count = (std::size_t)(std::thread::hardware_concurrency() / 2);
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
    config.target_time = 20;

    std::atomic<std::size_t> view_index{0};
    std::atomic<bool> view{false};

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
                    usleep(100000);
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
                    std::deque<uint8_t> nexts_1, nexts_2;
                    std::queue<uint8_t> ori_1 = player_1.next.queue;
                    std::queue<uint8_t> ori_2 = player_2.next.queue;
                    for (int i = 0; i < 6; ++i)
                    {
                        nexts_1.push_back(ori_1.front());
                        nexts_2.push_back(ori_2.front());
                        ori_1.pop();
                        ori_2.pop();
                    }
                    snprintf(out, sizeof out, "HOLD = %c NEXT = %c%c%c%c%c%c COMBO = %d B2B = %d IDX = %d, WIN = %d\n"
                                              "HOLD = %c NEXT = %c%c%c%c%c%c COMBO = %d B2B = %d IDX = %d, WIN = %d\n",
                             mino_to_char[player_1.next.hold], mino_to_char[nexts_1[0]], mino_to_char[nexts_1[1]], mino_to_char[nexts_1[2]], mino_to_char[nexts_1[3]], mino_to_char[nexts_1[4]], mino_to_char[nexts_1[5]], player_1.combo, player_1.b2b, match_result.first->id, win[0],
                             mino_to_char[player_2.next.hold], mino_to_char[nexts_2[0]], mino_to_char[nexts_2[1]], mino_to_char[nexts_2[2]], mino_to_char[nexts_2[3]], mino_to_char[nexts_2[4]], mino_to_char[nexts_2[5]], player_2.combo, player_2.b2b, match_result.second->id, win[1]);
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
                    //usleep(50000);
                };
                static int max_count = 1000;
                double b_stats = 0;
                while (win[0] < WIN_REQUIREMENT && win[1] < WIN_REQUIREMENT && std::abs(win[0] - win[1]) < 5)
                {
                    TetrisPlayer player_1(config, match_result.first->pos[PSO_CURRENT], dis, mess_dis, gen);
                    TetrisPlayer player_2(config, match_result.second->pos[PSO_CURRENT], dis, mess_dis, gen);
                    while (player_1.run() && player_2.run() && player_1.count < max_count)
                    {
                        int lines = std::min(player_1.cur_atk, player_2.cur_atk);
                        player_1.cur_atk -= lines;
                        player_2.cur_atk -= lines;
                        player_1.push_damage(player_2.cur_atk);
                        player_2.push_damage(player_1.cur_atk);
                        player_1.cur_atk = 0;
                        player_2.cur_atk = 0;
                        view_func(player_1, player_2);
                        if (f_generation != match_result.first->generation)
                        {
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
                    if (player_1.dead)
                    {
                        ++win[1];
                    }
                    else if (player_2.dead)
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
            } }));
    }
    while (true)
    {
        char input[256];
        std::cin.getline(input, 256);
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
            if ((size_t)id > s_mgr.swarm.size())
            {
                continue;
            }
            s_mgr.export_id(id);
            mtx.unlock();
        }
    }
    s_mgr.export_data();
}