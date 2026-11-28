// HeadlessMain.cpp — console runner that proves the engine works WITHOUT the GUI.
// Usage: runner --seed 42 --days 365 --pop 80
// Prints population, event count and the canonical stateHash, plus a few narration
// lines and one obituary (narration RNG only). Two runs with the same seed and
// commands print the SAME hash — that is the determinism proof.
#include "core/sim/Simulation.h"
#include "core/history/Narrator.h"
#include "tools/DemoSystem.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <inttypes.h>

using namespace tge;

int main(int argc, char** argv) {
    uint64_t seed = 42;
    int days = 365, pop = 80;
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--seed") && i + 1 < argc) seed = std::strtoull(argv[++i], nullptr, 10);
        else if (!std::strcmp(argv[i], "--days") && i + 1 < argc) days = std::atoi(argv[++i]);
        else if (!std::strcmp(argv[i], "--pop") && i + 1 < argc) pop = std::atoi(argv[++i]);
    }

    Simulation sim(seed, pop);
    DemoSystem demo;
    sim.addSystem(&demo);
    sim.run(days);

    const Snapshot& s = sim.snapshot();
    std::printf("seed=%" PRIu64 " days=%d initialPop=%d\n", seed, days, pop);
    std::printf("population: %d\n", s.population());
    std::printf("events: %zu\n", sim.history().size());
    std::printf("births=%d deaths=%d thefts=%d rumors=%d scandals=%d\n",
                s.metrics.births, s.metrics.deaths, s.metrics.thefts, s.metrics.rumors, s.metrics.scandals);
    std::printf("stateHash: 0x%016" PRIx64 "\n", sim.stateHash());

    Narrator nar;
    Rng& narr = sim.rngNarr();
    std::printf("\n-- recent narration --\n");
    size_t n = sim.history().size();
    size_t start = n > 5 ? n - 5 : 0;
    for (size_t i = start; i < n; ++i)
        std::printf("%s\n", nar.narrate(sim.history().get((EventId)i), sim.history(), sim.humans(), narr).c_str());

    for (size_t i = 0; i < sim.humans().size(); ++i) {
        if (!sim.humans()[i].alive) {
            std::printf("\n-- obituary --\n%s\n",
                        nar.obituary(sim.humans()[i].id, sim.history(), sim.humans(), narr).c_str());
            break;
        }
    }
    return 0;
}
