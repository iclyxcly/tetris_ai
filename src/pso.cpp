#include "engine.hpp"
#include "movegen.hpp"
#include "emu.hpp"
#include <random>
#include <stdio.h>
#include <vector>
#include <thread>
#include <mutex>
#include <iostream>
#include <atomic>

using namespace moenew;
using Playstyle = Evaluation::Playstyle;
struct PSOConfig
{
    double w;
    double dest_w;
    double c1;
    double c2;
    Playstyle x_max, x_min, v_max;
    PSOConfig()
    {
        w = 0.9;
        dest_w = 0.4;
        c1 = 2;
        c2 = 1;
        Playstyle p;
        auto pso_config = [&](int index, double value, double offset, double velocity)
        {
            x_max[index] = value + offset;
            x_min[index] = value - offset;
            v_max[index] = velocity;
        };
        auto &x = p;
        pso_config(Evaluation::ASPIN_1, x[Evaluation::ASPIN_1], 100, 5);
        pso_config(Evaluation::ASPIN_2, x[Evaluation::ASPIN_2], 100, 5);
        pso_config(Evaluation::ASPIN_3, x[Evaluation::ASPIN_3], 100, 5);
        pso_config(Evaluation::ASPIN_SLOT, x[Evaluation::ASPIN_SLOT], 10, 1);
        pso_config(Evaluation::COMBO, x[Evaluation::COMBO], 1000, 10);
        pso_config(Evaluation::ATTACK, x[Evaluation::ATTACK], 1000, 10);
        pso_config(Evaluation::CLEAR_1, x[Evaluation::CLEAR_1], 100, 5);
        pso_config(Evaluation::CLEAR_2, x[Evaluation::CLEAR_2], 100, 5);
        pso_config(Evaluation::CLEAR_3, x[Evaluation::CLEAR_3], 100, 5);
        pso_config(Evaluation::CLEAR_4, x[Evaluation::CLEAR_4], 100, 5);
        pso_config(Evaluation::B2B, x[Evaluation::B2B], 100, 5);
        pso_config(Evaluation::HEIGHT, x[Evaluation::HEIGHT], 1000, 10);
        pso_config(Evaluation::COL_TRANS, x[Evaluation::COL_TRANS], 1000, 10);
        pso_config(Evaluation::ROW_TRANS, x[Evaluation::ROW_TRANS], 1000, 10);
        pso_config(Evaluation::HOLE_COUNT, x[Evaluation::HOLE_COUNT], 1000, 10);
        pso_config(Evaluation::HOLE_LINE, x[Evaluation::HOLE_LINE], 1000, 10);
        pso_config(Evaluation::WIDE_2, x[Evaluation::WIDE_2], 1000, 10);
        pso_config(Evaluation::WIDE_3, x[Evaluation::WIDE_3], 1000, 10);
        pso_config(Evaluation::WIDE_4, x[Evaluation::WIDE_4], 1000, 10);
        pso_config(Evaluation::TANK_CLEAN, x[Evaluation::TANK_CLEAN], 100, 5);
        pso_config(Evaluation::CANCEL, x[Evaluation::CANCEL], 100, 5);
        pso_config(Evaluation::BUILD_ATTACK, x[Evaluation::BUILD_ATTACK], 100, 5);
        pso_config(Evaluation::SPIKE, x[Evaluation::SPIKE], 100, 5);
        pso_config(Evaluation::PENDING_LOCK, x[Evaluation::PENDING_LOCK], 100, 5);
    }
};
constexpr int WIN_REQUIREMENT = 15;
struct TetrisPlayer
{
    Playstyle param;
    Engine engine;
    Next real_next;
    Evaluation::Status status;
    int next;
    int count;
    int clear;
    int attack;
    int receive;

    void push_damage(uint8_t lines)
    {
        if (lines == 0)
        {
            return;
        }
        status.under_attack.push(lines, 1);
        receive += lines;
    }

    TetrisPlayer(Playstyle &param)
        : count(0), clear(0), attack(0), receive(0)
    {
        status.reset();
        engine.get_param() = param;
        this->param = param;
        auto &atk = engine.get_attack_table();
		atk.messiness = 0.05;
		atk.aspin_1 = 2;
		atk.aspin_2 = 4;
		atk.aspin_3 = 6;
		atk.clear_1 = 0;
		atk.clear_2 = 1;
		atk.clear_3 = 2;
		atk.clear_4 = 4;
		atk.pc = 10;
		atk.b2b = 1;
        atk.multiplier = 1;
		int combo_table[21] = {0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
		memcpy(atk.combo, combo_table, sizeof(atk.combo));
        next = 5;
    }

    bool run()
    {
        if (count > 200)
            if (count % 11 == 0)
            {
                engine.get_attack_table().multiplier += 0.1;
            }
        real_next.fill();
        status.next.next = real_next.get(next);
        status.next.hold = real_next.hold;
        status.next.fill();
        auto mino_loc = engine.get_mino_draft();
        engine.submit_form(mino_loc, status, true);
        auto result = engine.start_pso();
        status.under_attack.rngify();
        if (result.change_hold)
        {
            real_next.swap();
            status.next.swap();
        }
        if (!cycle(status, result, engine.get_attack_table()))
        {
            status.dead = true;
        }
        real_next.pop();
        ++count;
        attack += status.attack;
        clear += status.clear;
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
    Playstyle pos[4];
    PSOConfig cfg;
    int generation;
    double highscore;
    bool ingame;
    void inform_global_best(const Playstyle &global_best)
    {
        pos[PSO_BEST_GLOBAL] = global_best;
        if (id == 0)
        {
            pos[PSO_CURRENT] = global_best;
            pos[PSO_BEST_PERSONAL] = global_best;
        }
    }
    void push_new_standard(const Playstyle &best)
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

        for (int i = 0; i < Evaluation::END_OF_PARAM; ++i)
        {
            pos[PSO_VELOCITY][i] = (cfg.w * pos[PSO_VELOCITY][i]) + (cfg.c1 * r1 * (pos[PSO_BEST_PERSONAL][i] - pos[PSO_CURRENT][i])) + (cfg.c2 * r2 * (pos[PSO_BEST_GLOBAL][i] - pos[PSO_CURRENT][i]));
            pos[PSO_VELOCITY][i] = std::max(std::min(pos[PSO_VELOCITY][i], cfg.v_max[i]), -cfg.v_max[i]);
            pos[PSO_CURRENT][i] += pos[PSO_VELOCITY][i];
            pos[PSO_CURRENT][i] = std::max(std::min(pos[PSO_CURRENT][i], cfg.x_max[i]), cfg.x_min[i]);
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
    PSOParticleData(int id, Playstyle &param)
        : id(id), generation(0), ingame(false)
    {
        for (int i = 0; i < 4; ++i)
        {
            pos[i] = param;
        }
        if (id == 0)
        {
            return;
        }
        for (int i = 0; i < Evaluation::END_OF_PARAM; ++i)
        {
            pos[PSO_VELOCITY][i] = std::uniform_real_distribution<double>(-cfg.v_max[i], cfg.v_max[i])(engine);
            pos[PSO_CURRENT][i] = pos[PSO_CURRENT][i] + pos[PSO_VELOCITY][i];
        }
    }
    PSOParticleData() : id(0), generation(0), ingame(false)
    {
    }
};
struct PSOSwarmManager
{
    std::vector<PSOParticleData *> swarm;
    Playstyle global_best;

    void export_best(Playstyle &param)
    {
        FILE *file = fopen("best_param.txt", "w");
        if (file == nullptr)
        {
            return;
        }
        for (int i = 0; i < Evaluation::END_OF_PARAM; ++i)
        {
            fprintf(file, "%lf\n", param[i]);
        }
        fclose(file);
    }

    void import_init(Playstyle &param)
    {
        FILE *file = fopen("import_param.txt", "r");
        if (file == nullptr)
        {
            return;
        }
        for (int i = 0; i < Evaluation::END_OF_PARAM; ++i)
        {
            fscanf(file, "%lf", &param[i]);
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
        for (int i = 0; i < Evaluation::END_OF_PARAM; ++i)
        {
            fprintf(file, "%lf\n", particle->pos[PSO_CURRENT][i]);
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
        fwrite(&global_best, sizeof(Playstyle), 1, file);
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
        fread(&global_best, sizeof(Playstyle), 1, file);
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
        Playstyle param;
        import_init(param);
        for (int i = 0; i < count; ++i)
        {
            PSOParticleData particle(generate_new_id(), param);
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
    srand(time(nullptr));
    PSOSwarmManager s_mgr;
    if (!s_mgr.import_data())
    {
        s_mgr.init_pso(thread_count + 1);
    }
    std::recursive_mutex mtx;
    std::vector<std::thread> threads;
    std::atomic<std::size_t> view_index{0};
    std::atomic<bool> view{false};
    std::atomic<bool> force_win{false};

    for (std::size_t i = 0; i < thread_count; ++i)
    {
        threads.emplace_back(std::thread([&, i]()
                                         {
            std::size_t index = i + 1;
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
                    std::string nexts_1 = player_1.real_next.to_string();
                    std::string nexts_2 = player_2.real_next.to_string();
                    nexts_1.resize(6);
                    nexts_2.resize(6);
                    uint16_t up_1 = player_1.status.under_attack.total();
                    uint16_t up_2 = player_2.status.under_attack.total();
                    snprintf(out, sizeof out, "HOLD = %c NEXT = %s UP = %d COMBO = %d B2B = %d, APP = %3.2f, ID = %d, WIN = %d\n"
                                              "HOLD = %c NEXT = %s UP = %d COMBO = %d B2B = %d, APP = %3.2f, ID = %d, WIN = %d\n",
                             type_to_char((Piece)player_1.real_next.hold), nexts_1.c_str(), up_1, player_1.status.combo, player_1.status.b2b, (double)player_1.attack / (double)player_1.count, match_result.first->id, win[0],
                             type_to_char((Piece)player_2.real_next.hold), nexts_2.c_str(), up_2, player_2.status.combo, player_2.status.b2b, (double)player_2.attack / (double)player_2.count, match_result.second->id, win[1]);
                    Board map_copy1 = player_1.status.board;
                    Board map_copy2 = player_2.status.board;
                        map_copy1.paste(cache_get(player_1.status.next.peek(), DEFAULT_R, DEFAULT_X), DEFAULT_Y);
                        map_copy2.paste(cache_get(player_2.status.next.peek(), DEFAULT_R, DEFAULT_X), DEFAULT_Y);
                    for (int y = DEFAULT_Y + 4; y >= 0; --y)
                    {
                        snprintf(out + strlen(out), sizeof out - strlen(out), "%2d|", y);
                        for (int x = 0; x < 10; ++x)
                        {
                            strcat(out, map_copy1.get(x, y) ? box_1 : box_0);
                        }
                        snprintf(out + strlen(out), sizeof out - strlen(out), "|  %2d|", y);
                        for (int x = 0; x < 10; ++x)
                        {
                            strcat(out, map_copy2.get(x, y) ? box_1 : box_0);
                        }
                        strcat(out, "|\r\n");
                    }
                    printf("%s", out);
                };
                static int max_count = 1000;
                double b_stats = 0;
                while (win[0] < WIN_REQUIREMENT && win[1] < WIN_REQUIREMENT && (std::abs(win[0] - win[1]) < 5 || win[1] > win[0]))
                {
                    TetrisPlayer player_1(match_result.first->pos[PSO_CURRENT]);
                    TetrisPlayer player_2(match_result.second->pos[PSO_CURRENT]);
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
    exit(0);
}