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
constexpr int WIN_REQUIREMENT = 15;
constexpr int CANDIDATE_COUNT = 15;
struct TraceConfig
{
    Playstyle area;
    Playstyle pos, neg;
    int gen = 0;
    double sens = 1;
    TraceConfig()
    {
        memset(&area, 0, sizeof(area));
        memset(&pos, 0, sizeof(pos));
        memset(&neg, 0, sizeof(neg));
        area[Evaluation::CLEAR_1] = 30;
        area[Evaluation::CLEAR_2] = 30;
        area[Evaluation::CLEAR_3] = 30;
        area[Evaluation::CLEAR_4] = 30;
        area[Evaluation::ASPIN_1] = 30;
        area[Evaluation::ASPIN_2] = 30;
        area[Evaluation::ASPIN_3] = 30;
        area[Evaluation::ASPIN_SLOT] = 30;
        area[Evaluation::B2B] = 30;
        area[Evaluation::COMBO] = 30;
        area[Evaluation::ATTACK] = 30;
        area[Evaluation::CANCEL] = 30;
        area[Evaluation::TANK_CLEAN] = 30;
        area[Evaluation::HEIGHT] = 30;
        area[Evaluation::COL_TRANS] = 30;
        area[Evaluation::ROW_TRANS] = 30;
        area[Evaluation::HOLE_COUNT] = 30;
        area[Evaluation::HOLE_LINE] = 30;
        area[Evaluation::WIDE_2] = 30;
        area[Evaluation::WIDE_3] = 30;
        area[Evaluation::WIDE_4] = 30;
        area[Evaluation::BUILD_ATTACK] = 30;
        area[Evaluation::SPIKE] = 30;
        area[Evaluation::PENDING_LOCK] = 30;
        area[Evaluation::PENDING_HOLD] = 30;
    }
    void init(Playstyle &param)
    {
        for (int i = 0; i < Evaluation::END_OF_PARAM; ++i)
        {
            double max = area[i];
            double min = -area[i];
            if (!pos[i])
            {
                pos[i] = 1;
                param[i] = max;
            }
            else if (!neg[i])
            {
                neg[i] = 1;
                param[i] = min;
            }
            else
            {
                param[i] = min + (rand() % (int)(max - min + 1));
            }
        }
    }
    void newgen()
    {
        ++gen;
        sens *= 0.9;
    }
};

struct TetrisPlayer
{
    Engine engine;
    Next real_next;
    Evaluation::Status status;
    Random rand;
    int next_length;
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

    TetrisPlayer(Playstyle &param, uint64_t &seed)
        : count(0), clear(0), attack(0), receive(0), rand(seed)
    {
        status.reset();
        engine.get_param() = param;
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
        next_length = 5;
    }

    bool run()
    {
        if (count > 200)
            if (count % 11 == 0)
            {
                engine.get_attack_table().multiplier += 0.1;
            }
        real_next.fill();
        status.next.next = real_next.get(next_length);
        status.next.hold = real_next.hold;
        auto mino_loc = engine.get_mino_draft();
        engine.submit_form(mino_loc, status, true);
        auto result = engine.start_depth_thread();
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

struct Tracee
{
    int id;
    Playstyle base;
    Playstyle adjustment;
    bool pending[CANDIDATE_COUNT];
    bool ingame[CANDIDATE_COUNT];
    int wins;
    int count;
    void imitate(const Playstyle &ref)
    {
        Playstyle target = base;
        for (int i = 0; i < Evaluation::END_OF_PARAM; ++i)
        {
            adjustment[i] += ref.param[i] - base[i];
        }
        ++count;
    }
    void finalize(double sens)
    {
        for (int i = 0; i < Evaluation::END_OF_PARAM; ++i)
        {
            base[i] += (adjustment[i] / count) * sens;
        }
    }
    void reset()
    {
        memset(&adjustment, 0, sizeof(adjustment));
        wins = 0;
        count = 0;
    }
    Tracee(const Playstyle &base) : id(0), base(base), wins(0), count(0)
    {
        memset(&adjustment, 0, sizeof(adjustment));
        memset(&pending, 0, sizeof(pending));
        memset(&ingame, 0, sizeof(ingame));
    }
    Tracee() {}
    void insert_candidate(const int &id)
    {
        assert(!pending[id]);
        pending[id] = true;
    }
    void get_into_game(const int &id)
    {
        assert(pending[id]);
        pending[id] = false;
        ingame[id] = true;
    }
    void get_out_of_game(const int &id)
    {
        assert(ingame[id]);
        ingame[id] = false;
    }
    bool can_play()
    {
        for (int i = 0; i < CANDIDATE_COUNT; ++i)
        {
            if (pending[i])
            {
                return true;
            }
        }
        return false;
    }
    bool check(int id)
    {
        return !pending[id] || ingame[id];
    }
    bool complete()
    {
        for (int i = 0; i < CANDIDATE_COUNT; ++i)
        {
            if (pending[i] || ingame[i])
            {
                return false;
            }
        }
        return true;
    }
    int ingame_count()
    {
        int count = 0;
        for (int i = 0; i < CANDIDATE_COUNT; ++i)
        {
            if (ingame[i])
            {
                ++count;
            }
        }
        return count;
    }
    int pending_count()
    {
        int count = 0;
        for (int i = 0; i < CANDIDATE_COUNT; ++i)
        {
            if (pending[i])
            {
                ++count;
            }
        }
        return count;
    }
    int completed_count()
    {
        int count = 0;
        for (int i = 0; i < CANDIDATE_COUNT; ++i)
        {
            if (!pending[i] && !ingame[i])
            {
                ++count;
            }
        }
        return count - 1;
    }
};

struct Tracer
{
    std::vector<Tracee*> trace_data;
    TraceConfig config;
    struct ImitatorCompare
    {
        bool operator()(const Tracee &lhs, const Tracee &rhs)
        {
            return lhs.wins > rhs.wins;
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
    void export_id(const int id)
    {
        Tracee *trace = nullptr;
        for (auto &p : trace_data)
        {
            if (p->id == id)
            {
                trace = p;
                break;
            }
        }
        if (trace == nullptr)
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
            fprintf(file, "%lf\n", trace->base[i]);
        }
        fclose(file);
    }
    void export_data()
    {
        FILE *file = fopen("trace_data.bin", "wb");
        if (file == nullptr)
        {
            return;
        }
        fwrite(&config, sizeof(config), 1, file);
        for (auto &imitater : trace_data)
        {
            auto data = *imitater;
            for (int i = 0; i < CANDIDATE_COUNT; ++i)
            {
                data.pending[i] = data.pending[i] || data.ingame[i];
                data.ingame[i] = false;
            }
            fwrite(&data, sizeof(Tracee), 1, file);
        }
        fclose(file);
    }
    bool import_data()
    {
        FILE *file = fopen("trace_data.bin", "rb");
        if (file == nullptr)
        {
            return false;
        }
        fread(&config, sizeof(config), 1, file);
        while (true)
        {
            Tracee *tracee = new Tracee();
            if (fread(tracee, sizeof(Tracee), 1, file) != 1)
            {
                delete tracee;
                break;
            }
            trace_data.push_back(tracee);
        }
        fclose(file);
        return true;
    }
    void init(const int &size)
    {
        for (int i = 0; i < size; ++i)
        {
            Playstyle param;
            config.init(param);
            Tracee *tracee = new Tracee(param);
            tracee->id = generate_new_id();
            trace_data.push_back(tracee);
        }
        for (int i = 0; i < size; ++i)
        {
            for (int j = 0; j < size; ++j)
            {
                if (trace_data[i]->id != j)
                {
                    trace_data[i]->insert_candidate(j);
                }
            }
        }
    }
    std::pair<Tracee *, Tracee *> find_match_pair()
    {
        Tracee *a = nullptr;
        Tracee *b = nullptr;

        for (int i = trace_data.size() - 1; i >= 0; --i)
        {
            if (!trace_data[i]->can_play())
            {
                continue;
            }
            if (a == nullptr)
            {
                a = trace_data[i];
            }
            else if (b == nullptr)
            {
                if (trace_data[i]->check(a->id))
                {
                    continue;
                }
                b = trace_data[i];
                break;
            }
        }

        if (a != nullptr && b != nullptr)
        {
            a->get_into_game(b->id);
            b->get_into_game(a->id);
        }
        return std::make_pair(a, b);
    }
    int generate_new_id()
    {
        int id = -1;
        for (std::size_t i = 0; i < trace_data.size(); ++i)
        {
            if (id <= trace_data[i]->id)
            {
                id = trace_data[i]->id;
            }
        }
        return id + 1;
    }
    Tracee get_best()
    {
        Tracee best;
        best.wins = -1;
        for (auto &tracee : trace_data)
        {
            if (tracee->wins > best.wins)
            {
                best = *tracee;
            }
        }
        return best;
    }
    void newgen()
    {
        auto best = get_best();
        export_best(best.base);

        for (auto &tracee : trace_data)
        {
            tracee->finalize(config.sens);
            tracee->reset();
            for (int i = 0; i < CANDIDATE_COUNT; ++i)
            {
                if (tracee->id != i)
                {
                    tracee->insert_candidate(i);
                }
            }
        }
        config.newgen();
    }
    void inform_complete(std::pair<Tracee *, Tracee *> &pair, int win_1, int win_2)
    {
        pair.first->get_out_of_game(pair.second->id);
        pair.second->get_out_of_game(pair.first->id);
        int win_diff = win_1 - win_2;
        if (win_diff < 0)
        {
            for (int i = 0; i < -win_diff; ++i)
            {
                pair.first->imitate(pair.second->base);
                pair.second->imitate(pair.second->base);
            }
        }
        else if (win_diff > 0)
        {
            for (int i = 0; i < win_diff; ++i)
            {
                pair.first->imitate(pair.first->base);
                pair.second->imitate(pair.first->base);
            }
        }
        else
        {
            pair.first->imitate(pair.second->base);
            pair.second->imitate(pair.first->base);
        }
        export_data();
    }
    bool scan()
    {
        for (auto &tracee : trace_data)
        {
            if (!tracee->complete())
            {
                return false;
            }
        }
        return true;
    }
};

int main(void)
{
    srand(time(nullptr));
    Tracer tracer;
    if (!tracer.import_data())
    {
        tracer.init(CANDIDATE_COUNT);
    }
    std::recursive_mutex mtx;
    std::vector<std::thread> threads;
    std::atomic<std::size_t> view_index{0};
    std::atomic<bool> view{false};

    for (std::size_t i = 0; i < 32; ++i)
    {
        threads.emplace_back(std::thread([&, i]()
                                         {
            std::size_t index = i + 1;
            while (true)
            {
                mtx.lock();
                auto match_result = tracer.find_match_pair();
                if (match_result.first == nullptr || match_result.second == nullptr)
                {
                    if (tracer.scan())
                    {
                        tracer.newgen();
                    }
                    mtx.unlock();
                    Sleep(1000);
                    continue;
                }
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
                             type_to_char(player_1.real_next.hold), nexts_1.c_str(), up_1, player_1.status.combo, player_1.status.b2b, (double)player_1.attack / (double)player_1.count, match_result.first->id, win[0],
                             type_to_char(player_2.real_next.hold), nexts_2.c_str(), up_2, player_2.status.combo, player_2.status.b2b, (double)player_2.attack / (double)player_2.count, match_result.second->id, win[1]);
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
                while (win[0] < WIN_REQUIREMENT && win[1] < WIN_REQUIREMENT)
                {
                    uint64_t seed = rand();
                    TetrisPlayer player_1(match_result.first->base, seed);
                    TetrisPlayer player_2(match_result.second->base, seed);
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
                match_result.first->wins += win[0];
                match_result.second->wins += win[1];
                tracer.inform_complete(match_result, win[0], win[1]);
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
            }
        }
        if (input[0] == 'q')
        {
            break;
        }
        if (input[0] == 's')
        {
            mtx.lock();
            printf("GENERATION = %d\n", tracer.config.gen);
            for (auto &tracee : tracer.trace_data)
            {
                printf("ID = %4d, WINS = %4d, INGAME = %4zu, PEND = %4zu, COMPLETE = %4zu\n", tracee->id, tracee->wins, tracee->ingame_count(), tracee->pending_count(), tracee->completed_count());
            }
            mtx.unlock();
        }
        if (input[0] == 'e')
        {
            int id = atoi(input + 1);
            printf("EXPORTING ID = %d\n", id);
            mtx.lock();
            if ((std::size_t)id > tracer.trace_data.size())
            {
                continue;
            }
            tracer.export_id(id);
            mtx.unlock();
        }
    }
    tracer.export_data();
    exit(0);
}