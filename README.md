# The God Experiment

A deterministic, single-threaded society simulation for a DSA course project.
64×64 tile world, ~80 humans, **1 tick = 1 day**, fixed seeds. The pure-C++20 core
has **no Windows/MFC includes**; the MFC GUI (P4) only reads a `Snapshot` and pushes
`Command`s.

This repository currently contains the **P1 backbone**: the engine, the hand-written
DSA library, history/cause graph, narrator, and persistence. P2 (world/environment/
navigation) and P3 (humans/society) build on these contracts; minimal contract-faithful
placeholders for `World` and `Human` are included so the engine runs headlessly today.

## Folder layout
```
core/
  common/      Types, Rng, Fnv, Constants, Tuning            (P1)
  dsa/         DynArray HashTable MinHeap LinkedList Queue
               AvlTree Graph Sorting DsaStats               (P1, graded)
  sim/         Simulation, Command, Scheduler, Snapshot,
               ISystem, HeadlessMain                        (P1)
  history/     History, cause graph, Narrator, Grammar      (P1)
  persistence/ SaveLoad (nlohmann/json)                     (P1)
  world/       World contract + placeholder impl            (P2 owns)
  humans/      Human contract + placeholder                 (P3 owns)
gui/           MFC GUI                                      (P4, Windows only)
tests/unit/    doctest suites (100k-op randomized DSA tests)
tests/reference/  reference C PRNG for port verification
tools/         SeedHunt
data/scenarios/   demo.json (chosen demo seed)
third_party/   doctest v2.4.12, nlohmann/json v3.11.3 (single headers, MIT)
```

## Build & run (macOS / Linux, clang++ C++20)
```
make test          # build + run all unit tests
make run           # headless demo: runner --seed 42 --days 365 --pop 80
make               # build tests + runner + seedhunt
./build/seedhunt   # score seeds 1..200, print top 5
```

## Build & run (CMake, cross-platform incl. Windows/MSVC)
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/runner --seed 42 --days 365 --pop 80
```
The MFC GUI is added only on Windows (`if(WIN32)` in CMake).

## Determinism contract
Same seed + same command list ⇒ identical `stateHash()`. Save/load round-trips
preserve the hash. See `AGENTS.md` for the full rules and `reports/REPORT.md` for
verification output.

## Dependencies (pinned, MIT, single-header)
- doctest **v2.4.12** — `third_party/doctest/doctest.h`
- nlohmann/json **v3.11.3** — `third_party/nlohmann/json.hpp`
- RNG algorithms: xoshiro256** / splitmix64 (Blackman & Vigna, public domain).
