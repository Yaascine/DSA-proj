// SeedHunt.cpp — pick a lively demo seed (legitimate: all events are real sim output).
// Usage: seedhunt [maxSeed] [days] [pop]   (defaults 200 180 80)
// Scores each seed by event variety, deaths, scandals, rumor reach and corruption
// discovered, prints the top 5, and writes data/scenarios/demo.json with the winner.
#include "core/sim/Simulation.h"
#include "tools/DemoSystem.h"
#include <cstdio>
#include <cstdlib>
#include <inttypes.h>

using namespace tge;

struct Scored { uint64_t seed; int score; int variety; int deaths; int scandals; int rumors; int discovered; };

int main(int argc, char** argv) {
    int maxSeed = argc > 1 ? std::atoi(argv[1]) : 200;
    int days    = argc > 2 ? std::atoi(argv[2]) : 180;
    int pop     = argc > 3 ? std::atoi(argv[3]) : 80;

    Scored top[5];
    int topN = 0;
    for (int i = 0; i < 5; ++i) top[i] = Scored{0, -1, 0, 0, 0, 0, 0};

    for (int s = 1; s <= maxSeed; ++s) {
        Simulation sim((uint64_t)s, pop);
        DemoSystem demo;
        sim.addSystem(&demo);
        sim.run(days);

        bool seen[(int)EventType::COUNT] = {false};
        int discovered = 0;
        const auto& evs = sim.history().events();
        for (size_t i = 0; i < evs.size(); ++i) {
            seen[(int)evs[i].type] = true;
            if (evs[i].type == EventType::Discovered) ++discovered;
        }
        int variety = 0;
        for (int t = 0; t < (int)EventType::COUNT; ++t) if (seen[t]) ++variety;
        const Metrics& m = sim.metrics();
        int score = variety * 10 + m.deaths * 2 + m.scandals * 5 + m.rumors + discovered * 8;

        Scored cur{(uint64_t)s, score, variety, m.deaths, m.scandals, m.rumors, discovered};
        // insert into top-5 (descending score, tiebreak by smaller seed)
        int pos = topN;
        for (int i = 0; i < topN; ++i)
            if (cur.score > top[i].score || (cur.score == top[i].score && cur.seed < top[i].seed)) { pos = i; break; }
        if (pos < 5) {
            for (int i = (topN < 5 ? topN : 4); i > pos; --i) top[i] = top[i - 1];
            top[pos] = cur;
            if (topN < 5) ++topN;
        }
    }

    std::printf("Top %d seeds (days=%d pop=%d):\n", topN, days, pop);
    std::printf("rank seed   score variety deaths scandals rumors discovered\n");
    for (int i = 0; i < topN; ++i)
        std::printf("%2d   %4" PRIu64 "  %5d %6d %6d %7d %6d %8d\n",
                    i + 1, top[i].seed, top[i].score, top[i].variety, top[i].deaths,
                    top[i].scandals, top[i].rumors, top[i].discovered);

    if (topN > 0) {
        FILE* f = std::fopen("data/scenarios/demo.json", "w");
        if (f) {
            std::fprintf(f, "{\n  \"schemaVersion\": 1,\n  \"seed\": %" PRIu64 ",\n  \"days\": %d,\n  \"pop\": %d,\n  \"score\": %d,\n  \"chosenBy\": \"SeedHunt\"\n}\n",
                         top[0].seed, days, pop, top[0].score);
            std::fclose(f);
            std::printf("\nWrote data/scenarios/demo.json (seed %" PRIu64 ")\n", top[0].seed);
        }
    }
    return 0;
}
