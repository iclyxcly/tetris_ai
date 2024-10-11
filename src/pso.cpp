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
        c2 = 2;
        Playstyle p;
        auto pso_config = [&](int index, double offset, double velocity)
        {
            x_max[index] = p[index] + offset;
            x_min[index] = p[index] - offset;
            v_max[index] = velocity;
        };
        pso_config(Evaluation::HEIGHT, 1000, 50);
        pso_config(Evaluation::HOLE_LINE, 1000, 50);
        pso_config(Evaluation::WIDE_2, 1000, 50);
        pso_config(Evaluation::WIDE_3, 1000, 50);
        pso_config(Evaluation::WIDE_4, 1000, 50);
        pso_config(Evaluation::COMBO, 1000, 50);
        pso_config(Evaluation::BUILD_ATTACK, 1000, 50);
        pso_config(Evaluation::COL_TRANS, 500, 10);
        pso_config(Evaluation::ROW_TRANS, 500, 10);
        pso_config(Evaluation::HOLE_COUNT, 500, 10);
        pso_config(Evaluation::AGGREGATE_HEIGHT, 500, 10);
        pso_config(Evaluation::BUMPINESS, 500, 10);
        pso_config(Evaluation::ASPIN_SLOT, 500, 10);
        pso_config(Evaluation::ATTACK, 133.3, 6.7);
        pso_config(Evaluation::SPIKE, 133.3, 6.7);
        pso_config(Evaluation::TANK_CLEAN, 133.3, 6.7);
        pso_config(Evaluation::PENDING_HOLD, 133.3, 6.7);
        pso_config(Evaluation::CANCEL, 133.3, 6.7);
        pso_config(Evaluation::PENDING_LOCK, 20, 1);
        pso_config(Evaluation::B2B, 4000, 200);
        pso_config(Evaluation::ASPIN_1, 800, 40);
        pso_config(Evaluation::ASPIN_2, 800, 40);
        pso_config(Evaluation::ASPIN_3, 800, 40);
        pso_config(Evaluation::CLEAR_1, 800, 40);
        pso_config(Evaluation::CLEAR_2, 800, 40);
        pso_config(Evaluation::CLEAR_3, 800, 40);
        pso_config(Evaluation::CLEAR_4, 800, 40);
    }
};
constexpr int MAX_MATCH = 55;
constexpr int MATCH_AREA = 4;
struct TetrisPlayer
{
    Playstyle param;
    Engine engine;
    Next real_next;
    Evaluation::Status status;
    Random rand;
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
        bool push_1 = !status.under_attack.empty() || !(rand.next() % 3) || status.clear != 0;
        status.under_attack.push(lines, push_1);
        receive += lines;
    }

    TetrisPlayer(Playstyle &param, uint64_t seed)
        : count(0), clear(0), attack(0), receive(0), rand(seed)
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
        auto mino_loc = engine.get_mino_draft();
        engine.submit_form(mino_loc, status, true);
        auto result = engine.start_depth();
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
constexpr double ELO_DEFAULT = 1500.0;
void calc_elo(double &a, double &b, int a_match, int b_match, int max_match, double a_wins)
{
    double ea = 1.0 / (1.0 + pow(10.0, (double)-(a - b) / 400.0));
    double eb = 1.0 / (1.0 + pow(10.0, (double)-(b - a) / 400.0));
    a = a + (36.0 * (double)(max_match - a_match) / max_match + 4.0) * ((double)a_wins - ea);
    b = b + (36.0 * (double)(max_match - b_match) / max_match + 4.0) * (!(double)a_wins - eb);
}
struct PSOParticleData
{
    int id;
    Playstyle pos[4];
    PSOConfig cfg;
    int gen;
    int match;
    int ingame;
    double rating;
    double best;
    void inform_global_best(const PSOParticleData &best)
    {
        pos[PSO_BEST_GLOBAL] = best.pos[PSO_BEST_PERSONAL];
        if (id == 0)
        {
            pos[PSO_CURRENT] = best.pos[PSO_BEST_PERSONAL];
            pos[PSO_BEST_PERSONAL] = best.pos[PSO_BEST_PERSONAL];
            this->best = best.best;
        }
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
            cfg.w -= 0.01;
        }
    }
    void advance()
    {
        if (rating > best)
        {
            best = rating;
            pos[PSO_BEST_PERSONAL] = pos[PSO_CURRENT];
        }
        else 
        {
            best = best * 0.95 + rating * 0.05;
        }
        ++gen;
        rating = ELO_DEFAULT;
        match = 0;
        calc_init();
    }
    void inform_complete()
    {
        ++match;
        --ingame;
        if (match >= MAX_MATCH)
        {
            advance();
        }
    }
    PSOParticleData(int id, Playstyle &param)
        : id(id), best(0), gen(0), match(0), ingame(0), rating(ELO_DEFAULT) 
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
    PSOParticleData() : id(0), best(0), gen(0), match(0), ingame(0), rating(ELO_DEFAULT) {}
};
struct PSOSwarmManager
{
    std::vector<PSOParticleData *> swarm;
    Playstyle global_best;

    struct PSOSort
    {
        bool operator()(PSOParticleData *a, PSOParticleData *b)
        {
            return a->rating > b->rating;
        }
    };

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
            particle_data.ingame = 0;
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
        return (a->gen * MAX_MATCH + a->match + a->ingame) < (b->gen * MAX_MATCH + b->match + b->ingame);
    }

    std::pair<PSOParticleData *, PSOParticleData *> find_match_pair()
    {
        std::sort(swarm.begin(), swarm.end(), PSOSort());
        PSOParticleData *a = nullptr;
        int a_idx = -1;
        PSOParticleData *b = nullptr;

        for (int i = swarm.size() - 1; i >= 0; --i)
        {
            if (a == nullptr)
            {
                a = swarm[i];
                a_idx = i;
                continue;
            }
            if (matchcmp(swarm[i], a))
            {
                a = swarm[i];
                a_idx = i;
            }
        }
        std::vector<PSOParticleData *> match_area;
        int begin = std::max(0, a_idx - MATCH_AREA);
        int end = std::min(static_cast<int>(swarm.size()) - 1, a_idx + MATCH_AREA);
        for (int i = begin; i < end; ++i)
        {
            if (i == a_idx)
            {
                continue;
            }
            match_area.push_back(swarm[i]);
        }
        b = match_area[std::uniform_int_distribution<int>(0, match_area.size() - 1)(engine)];
        a->ingame++;
        b->ingame++;
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
            if (particle->best > best.best)
            {
                best = *particle;
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
                particle->inform_global_best(best);
            }
            export_best(global_best);
        }
    }

    void inform_complete(PSOParticleData *a, PSOParticleData *b, bool a_win)
    {
        calc_elo(a->rating, b->rating, a->match, b->match, MAX_MATCH, a_win);
        a->inform_complete();
        b->inform_complete();
        export_data();
        update_best();
    }
};

int main(void)
{
    const std::size_t thread_count = std::thread::hardware_concurrency();
    srand(time(nullptr));
    PSOSwarmManager s_mgr;
    if (!s_mgr.import_data())
    {
        s_mgr.init_pso(50);
    }
    std::recursive_mutex mtx;
    std::vector<std::thread> threads;
    std::atomic<std::size_t> view_index{0};
    std::atomic<double> view_rating{0};
    std::atomic<bool> view{false};

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
                mtx.unlock();
                auto view_func = [&](TetrisPlayer &player_1, TetrisPlayer &player_2)
                {
                    if (view && (view_rating < match_result.first->rating || view_rating < match_result.second->rating))
                    {
                        mtx.lock();
                        if (view && (view_rating < match_result.first->rating || view_rating < match_result.second->rating))
                        {
                            view_rating = std::max(match_result.first->rating, match_result.second->rating);
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
                    snprintf(out, sizeof out, "HOLD = %c NEXT = %s UP = %d COMBO = %d B2B = %d, APP = %3.2f, ID = %d, ELO = %.2f\n"
                                              "HOLD = %c NEXT = %s UP = %d COMBO = %d B2B = %d, APP = %3.2f, ID = %d, ELO = %.2f\n",
                             type_to_char(player_1.real_next.hold), nexts_1.c_str(), up_1, player_1.status.combo, player_1.status.b2b, (double)player_1.attack / (double)player_1.count, match_result.first->id, match_result.first->rating,
                             type_to_char(player_2.real_next.hold), nexts_2.c_str(), up_2, player_2.status.combo, player_2.status.b2b, (double)player_2.attack / (double)player_2.count, match_result.second->id, match_result.second->rating);
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
                uint64_t seed = rand();
                TetrisPlayer player_1(match_result.first->pos[PSO_CURRENT], seed);
                TetrisPlayer player_2(match_result.second->pos[PSO_CURRENT], seed);
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
                }
                bool a_win;
                if (player_1.status.dead)
                {
                    a_win = false;
                }
                else if (player_2.status.dead)
                {
                    a_win = true;
                }
                else
                {
                    double app_1 = (double)player_1.attack / (double)player_1.clear;
                    double app_2 = (double)player_2.attack / (double)player_2.clear;
                    if (app_1 > app_2)
                    {
                        a_win = true;
                    }
                    else if (app_2 > app_1)
                    {
                        a_win = false;
                    }
                    else
                    {
                        a_win = dist(engine) > 0.5;
                    }
                }
                mtx.lock();
                s_mgr.inform_complete(match_result.first, match_result.second, a_win);
                if (view && view_index == index)
                {
                    view_index = 0;
                    view_rating = 0;
                }
                mtx.unlock();
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
                view_rating = 0;
            }
        }
        if (input[0] == 'q')
        {
            break;
        }
        if (input[0] == 'r')
        {
            mtx.lock();
            for (auto &particle : s_mgr.swarm)
            {
                printf("ID = %4d RATING = %7.2f, BEST = %7.2f, GEN = %4d, MATCH = %4d, INGAME = %3d\n", particle->id, particle->rating, particle->best, particle->gen, particle->match, particle->ingame);
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
    }
    s_mgr.export_data();
    exit(0);
}