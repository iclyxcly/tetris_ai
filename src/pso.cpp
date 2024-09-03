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
        c1 = 2.0;
        c2 = 2.0;
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
constexpr int MAX_MATCHES = 32;
constexpr double ELO_DEFAULT = 1500.0;
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
void calc_elo(double &a, double &b, bool a_wins)
{
    double ea = 1.0 / (1.0 + std::pow(10.0, (b - a) / 400.0));
    double eb = 1.0 / (1.0 + std::pow(10.0, (a - b) / 400.0));
    double sa = a_wins ? 1.0 : 0.0;
    double sb = a_wins ? 0.0 : 1.0;
    const double K = 32.0;
    a = a + K * (sa - ea);
    b = b + K * (sb - eb);
}
struct PSOParticleData
{
    int id;
    TetrisParam pos[4];
    PSOConfig cfg;
    double best_score;
    double score;
    int generation;
    int matches;
    int concurrent_matches;
    void inform_global_best(const TetrisParam &global_best)
    {
        pos[PSO_BEST_GLOBAL] = global_best;
        if (id == 0)
        {
            pos[PSO_CURRENT] = global_best;
            pos[PSO_BEST_PERSONAL] = global_best;
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
    void advance()
    {
        if (score > best_score)
        {
            best_score = score;
            pos[PSO_BEST_PERSONAL] = pos[PSO_CURRENT];
        }
        else 
        {
            best_score = best_score * 0.95 + score * 0.05;
        }
        ++generation;
        score = ELO_DEFAULT;
        matches = 0;
        concurrent_matches = 0;
        calc_init();
    }
    void inform_complete()
    {
        --concurrent_matches;
        ++matches;
        if (matches >= MAX_MATCHES)
        {
            advance();
        }
    }
    PSOParticleData(int id)
        : id(id), best_score(0), score(ELO_DEFAULT), generation(0), matches(0), concurrent_matches(0)
    {
        if (id == 0)
        {
            return;
        }
        for (int i = 0; i < END_OF_PARAM; ++i)
        {
            pos[PSO_CURRENT].weight[i] = std::uniform_real_distribution<double>(cfg.x_min.weight[i], cfg.x_max.weight[i])(engine);
            pos[PSO_BEST_PERSONAL].weight[i] = pos[PSO_CURRENT].weight[i];
            pos[PSO_VELOCITY].weight[i] = std::uniform_real_distribution<double>(-cfg.v_max.weight[i], cfg.v_max.weight[i])(engine);
        }
    }
    PSOParticleData() : id(0), best_score(0), score(ELO_DEFAULT), generation(0), matches(0), concurrent_matches(0)
    {
    }
};
struct PSOSwarmManager
{
    std::vector<PSOParticleData *> swarm;
    TetrisParam global_best;
    struct PSOSwarmSorter
    {
        bool operator()(const PSOParticleData *a, const PSOParticleData *b)
        {
            return a->score > b->score;
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
            particle_data.concurrent_matches = 0;
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
        std::sort(swarm.begin(), swarm.end(), PSOSwarmSorter());
    }

    bool matchcmp(PSOParticleData *a, PSOParticleData *b)
    {
        int a_matches = a->matches + a->concurrent_matches + (a->generation * MAX_MATCHES);
        int b_matches = b->matches + b->concurrent_matches + (b->generation * MAX_MATCHES);
        return a_matches < b_matches;
    }

    std::pair<PSOParticleData *, PSOParticleData *> find_match_pair()
    {
        PSOParticleData *a = swarm[0];
        std::size_t index = 0;
        for (int i = swarm.size() - 1; i >= 0; --i)
        {
            if (matchcmp(swarm[i], a))
            {
                a = swarm[i];
                index = i;
            }
        }
        PSOParticleData *b = index + 1 < swarm.size() ? swarm[index + 1] : swarm[index - 1];
        ++a->concurrent_matches;
        ++b->concurrent_matches;
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
        best.best_score = std::numeric_limits<double>::min();
        for (auto &particle : swarm)
        {
            if (particle->best_score > best.best_score)
            {
                best = *particle;
            }
        }
        return best;
    }

    void insert_new(const TetrisParam &param)
    {
        PSOParticleData particle;
        particle.id = generate_new_id();
        particle.pos[PSO_BEST_PERSONAL] = param;
        particle.pos[PSO_BEST_GLOBAL] = global_best;
        particle.calc_init();
        swarm.push_back(new PSOParticleData(particle));
        std::sort(swarm.begin(), swarm.end(), PSOSwarmSorter());
    }

    void insert_copy()
    {
        auto best = get_best();
        best.id = generate_new_id();
        swarm.push_back(new PSOParticleData(best));
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
                particle->inform_global_best(global_best);
            }
            export_best(global_best);
        }
    }

    void inform_complete(PSOParticleData *a, PSOParticleData *b, const int &gen_1, const int &gen_2, bool a_win)
    {
        bool a_cancelled = a->generation != gen_1;
        bool b_cancelled = b->generation != gen_2;

        if (a_cancelled && b_cancelled)
        {
            return;
        }
        else if (a_cancelled)
        {
            b->inform_complete();
            return;
        }
        else if (b_cancelled)
        {
            a->inform_complete();
            return;
        }
        else {
            a->inform_complete();
            b->inform_complete();
        }
        calc_elo(a->score, b->score, a_win);
        std::sort(swarm.begin(), swarm.end(), PSOSwarmSorter());
        update_best();
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
                int generation_1 = match_result.first->generation;
                int generation_2 = match_result.second->generation;
                mtx.unlock();
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
                    snprintf(out, sizeof out, "HOLD = %c NEXT = %c%c%c%c%c%c COMBO = %d B2B = %d IDX = %d, THREAD = %ld\n"
                                              "HOLD = %c NEXT = %c%c%c%c%c%c COMBO = %d B2B = %d IDX = %d\n",
                             mino_to_char[player_1.next.hold], mino_to_char[nexts_1[0]], mino_to_char[nexts_1[1]], mino_to_char[nexts_1[2]], mino_to_char[nexts_1[3]], mino_to_char[nexts_1[4]], mino_to_char[nexts_1[5]], player_1.combo, player_1.b2b, match_result.first->id, index,
                             mino_to_char[player_2.next.hold], mino_to_char[nexts_2[0]], mino_to_char[nexts_2[1]], mino_to_char[nexts_2[2]], mino_to_char[nexts_2[3]], mino_to_char[nexts_2[4]], mino_to_char[nexts_2[5]], player_2.combo, player_2.b2b, match_result.second->id);
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
                int win[2] = {0, 0};
                while (win[0] < 7 && win[1] < 7)
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
                        if (generation_1 != match_result.first->generation || generation_2 != match_result.second->generation)
                        {
                            break;
                        }
                    }
                    if (generation_1 != match_result.first->generation || generation_2 != match_result.second->generation)
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
                    s_mgr.inform_complete(match_result.first, match_result.second, generation_1, generation_2, win[0] > win[1]);
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
        if (input[0] == 'r')
        {
            mtx.lock();
            for (auto &particle : s_mgr.swarm)
            {
                printf("ID = %4d SCORE = %7.2f, HIGH = %7.2f, GEN = %4d, MATCH = %4d, INGAME = %3d\n", particle->id, particle->score, particle->best_score, particle->generation, particle->matches, particle->concurrent_matches);
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