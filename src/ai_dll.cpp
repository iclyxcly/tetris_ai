#ifdef _WIN32
#define DECLSPEC_EXPORT __declspec(dllexport)
#define WINAPI __stdcall
#define strdup _strdup
#else
#define DECLSPEC_EXPORT
#define WINAPI
#define __cdecl
#endif

#define EVAL_ID 1
#define AI_DLL_VERSION 3

#include "engine.hpp"
#include "pathgen.hpp"
#include "eval.hpp"
#include "utils.hpp"

using namespace moenew;

Engine engine;
Evaluation::Status status;

void debug(const std::string &message)
{
    FILE *file = fopen("debug.txt", "w");
    if (file)
    {
        fprintf(file, "%s\n", message.c_str());
        fclose(file);
    }
}

extern "C" DECLSPEC_EXPORT int __cdecl AIDllVersion()
{
    return AI_DLL_VERSION;
}

extern "C" DECLSPEC_EXPORT char *__cdecl AIName(int level)
{
    std::string name = "(Param) Stable Threaded LV ";
    name += std::to_string(level);
    return strdup(name.c_str());
}

extern "C" DECLSPEC_EXPORT void __cdecl NewGame()
{
    debug("ok");
    auto &attackTable = engine.get_attack_table();
    attackTable.messiness = 0.05;
    attackTable.aspin_1 = 2;
    attackTable.aspin_2 = 4;
    attackTable.aspin_3 = 6;
    attackTable.clear_1 = 0;
    attackTable.clear_2 = 1;
    attackTable.clear_3 = 2;
    attackTable.clear_4 = 4;
    attackTable.pc = 10;
    attackTable.b2b = 1;
    attackTable.multiplier = 1;
    int comboTable[21] = {0, 0, 0, 1, 1, 1, 2, 2, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
    memcpy(attackTable.combo, comboTable, sizeof(attackTable.combo));
    status.reset();
}

void read_config(Evaluation::Playstyle &param)
{
    FILE *file = fopen("best_param_2.txt", "r");
    if (file)
    {
        for (int i = 0; i < Evaluation::END_OF_PARAM; ++i)
        {
            fscanf(file, "%lf\n", &param[i]);
        }
        fclose(file);
    }
    else
    {
        utils::println(utils::ERR, " -> Failed to open best_param.txt");
    }
}

extern "C" DECLSPEC_EXPORT char *__cdecl TetrisAI(int overfield[], int field[], int field_w, int field_h, int b2b, int combo, char next[], char hold, bool curCanHold, char active, int x, int y, int spin, bool canhold, bool can180spin, int upcomeAtt[], int comboTable[], int maxDepth, int level, int player)
{
    read_config(engine.get_param());
    status.board.clear();
    for (size_t d = 0, s = 22; d < 23; ++d, --s)
    {
        status.board.field[d] = field[s];
    }
    status.board.tidy();
    status.b2b = b2b > 0;
    status.combo = combo;
    status.next.reset();
    status.next.next = char_to_type(active);
    for (int i = 0; i < maxDepth; ++i)
    {
        status.next.next += char_to_type(next[i]);
    }
    status.next.hold = char_to_type(hold);
    status.under_attack.clear();
    status.under_attack.push(upcomeAtt[0], 0);
    status.under_attack.push(upcomeAtt[1], 1);
    engine.submit_form(engine.get_mino_draft(), status, canhold, EVAL_ID);
    static double const base_time = std::pow(100, 1.0 / 8);
    auto raw_result = engine.start_threaded(std::pow(base_time, level));
    auto result = raw_result.decision;
    if (result.change_hold)
    {
        status.next.swap();
    }
    PathGen pathgen(status.board, engine.get_mino_draft(), result, status.next.peek());
    auto path_str = (result.change_hold ? "v" : "") + pathgen.build() + "V";
    status = raw_result.status;
    return strdup(path_str.c_str());
}