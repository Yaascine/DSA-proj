# The God Experiment

A deterministic, single-threaded society simulation for a **DSA course project** (4-person team).
64×64 tile world, ~80 humans, **1 tick = 1 day**, fixed seeds. The pure-C++20 core has
**no Windows/MFC includes**; the MFC GUI (P4) only reads a `Snapshot` and pushes `Command`s.

> ## 🚧 Project status — this is **P1**
> This repository currently contains the **P1 backbone**, complete and tested:
> the engine, the hand-written DSA library, history/cause graph, narrator, and persistence.
> **57/57 unit tests pass.** P2, P3 and P4 are **not built yet** — see the ownership table.
> Teammates: build on the frozen contracts below; don't change them without whole-team OK.

## Who owns what
| Part | Folder | Owner | Status |
|---|---|---|---|
| Common, DSA library, sim engine, history, narrator, persistence | `core/common`, `core/dsa`, `core/sim`, `core/history`, `core/persistence` | **P1** | ✅ Done |
| World, environment, navigation (BFS/Dijkstra/A*) | `core/world`, `core/environment`, `core/navigation` | **P2** | ⬜ Placeholder only |
| Humans, society (attributes, decisions, leaders, rumors) | `core/humans`, `core/society` | **P3** | ⬜ Placeholder only |
| MFC GUI (Windows, GDI) | `gui/` | **P4** | ⬜ Not started |

**Placeholders to replace:** `core/world/World.*` (worldgen + empty `findPath`),
`core/humans/Human.h` (basic attributes only), and `tools/DemoSystem.h` (a demo life-system
so the engine runs headlessly today). P2/P3 replace these with the real implementations.

## What P1 gives you to build on
- **Frozen contracts:** `core/common/Types.h`, `core/world/World.h`, `core/humans/Human.h`,
  `core/history/History.h`, `core/sim/Command.h`, `core/sim/Snapshot.h`, `core/sim/ISystem.h`.
- **Hand-written DSA library** (`core/dsa/`): DynArray, HashTable, MinHeap, LinkedList, Queue,
  AvlTree, Graph, Sorting — use these (no STL containers for the graded structures). Each
  carries `DsaStats` counters for the diagnostics panel.
- **Deterministic engine:** register an `ISystem`, push `Command`s, read `Snapshot`.
  Same seed + same commands ⇒ identical `stateHash()`.

## Build & run
**macOS / Linux (verified, clang++ C++20):**
```
make test          # build + run all 57 unit tests
make run           # headless demo: runner --seed 42 --days 365 --pop 80
make               # build tests + runner + seedhunt
./build/seedhunt   # score seeds 1..200, pick a demo seed
```
**Windows / cross-platform (CMake; P4 uses this for MFC):**
```
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```
The MFC GUI is added only on Windows (`if(WIN32)` in CMake). See `docs/PORTABILITY.md`.

## Team workflow (read this first!)
👉 **`docs/GIT_TEAM_GUIDE.md`** — simple branch/PR steps for everyone.
- `main` always works; never code directly on `main`.
- One task = one branch → Pull Request → CI must pass → **P1 merges**.
- **Contracts are frozen.** Changing a shared header needs whole-team agreement.
- CI (`.github/workflows/ci.yml`) runs the tests + a determinism check on every push/PR.

## Determinism rules (do not break)
Single thread; own RNG only (`core/common/Rng.h`, four streams); no `unordered_map`
iteration in sim logic; tiebreakers everywhere; integer sim values; entities reference by
ID. Full rules in **`AGENTS.md`**.

## Folder layout
```
core/
  common/      Types, Rng, Fnv, Constants, Tuning            (P1)
  dsa/         DynArray HashTable MinHeap LinkedList Queue
               AvlTree Graph Sorting DsaStats               (P1, graded)
  sim/         Simulation, Command, Scheduler, Snapshot,
               ISystem, HeadlessMain, WealthMetrics         (P1)
  history/     History, cause graph, Narrator, Grammar      (P1)
  persistence/ SaveLoad (nlohmann/json)                     (P1)
  world/       World contract + placeholder impl            (P2 owns)
  humans/      Human contract + placeholder                 (P3 owns)
gui/           MFC GUI                                      (P4, Windows only)
tests/unit/    doctest suites (100k-op randomized DSA tests)
tests/reference/  reference C PRNG for port verification
tools/         SeedHunt, DemoSystem (demo-only)
data/scenarios/   demo.json (chosen demo seed)
docs/          GIT_TEAM_GUIDE.md, PORTABILITY.md
reports/       REPORT.md + verification logs
third_party/   doctest v2.4.12, nlohmann/json v3.11.3 (MIT, single headers)
```

## Dependencies (pinned, MIT, single-header)
- doctest **v2.4.12** — `third_party/doctest/doctest.h`
- nlohmann/json **v3.11.3** — `third_party/nlohmann/json.hpp`
- RNG: xoshiro256** / splitmix64 (Blackman & Vigna, public domain), verified vs reference C.

## Reports
- `reports/REPORT.md` — full P1 build + hardening report.
- `reports/*.txt` — real test/determinism/seed-hunt output.
