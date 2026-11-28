# P1 Build Report — The God Experiment

**Role:** P1 (engine, DSA library, history, narrator, persistence)
**Date:** 2026-11-28
**Machine:** macOS, Apple clang 21.0.0, CMake 4.5.1, GNU Make 3.81
**Status:** ✅ Complete and verified, then hardened (Phase 0 fixes below).
**57/57 test cases pass (2,608,583 assertions).** Both Makefile and CMake builds green.
Determinism confirmed across optimization levels.

---

## 0. Phase 0 hardening update (2026-11-28, after the initial build)

A self-review found real flaws; these were fixed and proven with new regression tests.
**This section supersedes the older figures in §§4–5 (which described the first build).**

| Fix | What changed | Proof |
|---|---|---|
| **Duplicate-ID bug** | New humans now come from ONE authority (`Simulation::birthHuman` / `nextHumanId_`). The demo no longer assigns IDs from `humans.size()`, so births + God-Mode spawns can't collide. | `IdUniquenessTest` (fails on the old code) |
| **DynArray lifetime** | Rewrote with raw storage + placement-new + explicit destructors, so non-trivial elements (e.g. names) release resources immediately and ctor/dtor counts balance. | `DynArrayLifetimeTest` |
| **HashTable shrink** | Buckets halve when load drops below 0.25 (min 8), giving memory back after bulk erases. | `RobustnessTest` |
| **RNG guards** | `range` handles a reversed range safely; `chance` clamps to 0..100 (RNG consumption unchanged). | `RobustnessTest` |
| **Test gaps filled** | Added random graph node/edge ops, and save/load *with* commands. | `GraphRandomTest`, `PersistenceCommandsTest` |
| **CI + team workflow** | Added `.github/workflows/ci.yml` (tests + determinism smoke check on every push/PR), `docs/GIT_TEAM_GUIDE.md`, `docs/PORTABILITY.md`. | — |

**Updated verification:**
```
[doctest] test cases:      57 |      57 passed | 0 failed | 0 skipped
[doctest] assertions: 2608583 | 2608583 passed | 0 failed |
[doctest] Status: SUCCESS!
```
- Determinism (seed 42, 365 days): Makefile `-O2` run1 = run2 = CMake `-O3 -DNDEBUG`
  = **`0xc2ca09c5c57d7731`**.
- Note: this fingerprint differs from the first build (`0x6b30f08a55667c80`) because the
  birth fix changed the order of two RNG draws. **Determinism is unaffected** — the same
  seed still reproduces exactly; only the demo's specific sequence shifted.
- Refreshed seed hunt → demo seed **17** (`data/scenarios/demo.json`).
- `core` confirmed free of `<windows.h>`, MFC, `<unistd.h>`, `__attribute__`, `M_PI`
  (MSVC-ready by inspection; actual MSVC compile is P4's early-Windows task).

**Scope note:** per the team decision, this round was **P1 fixes only** — P2 (world),
P3 (people), P4 (MFC GUI) remain for teammates; no Mac GUI fallback (MFC-only).

---

## 1. Summary

The full P1 backbone from `P1_Engine_DSA_History.md` is implemented, tested, and
committed. Everything is pure C++20 with **no Windows/MFC includes** in `core`. The
graded data structures are 100% hand-written (no STL containers inside them), each
carrying real `DsaStats` operation counters. The engine runs headlessly today via a
demo life-system, and the same seed always produces the same canonical `stateHash`.

| Area | Files | Lines |
|---|---|---|
| `core/` (common, dsa, sim, history, persistence, world, humans) | 36 | ~2,759 |
| `tests/` (16 unit suites + reference C PRNG) | 17 | ~861 |
| `tools/` (DemoSystem, SeedHunt) | 2 | ~195 |

Third-party (pinned, MIT, single-header): **doctest v2.4.12**, **nlohmann/json v3.11.3**.
RNG algorithms: xoshiro256** / splitmix64 (Blackman & Vigna, public domain), ported and
verified against the reference C.

---

## 2. What was built

### 2.1 `core/common/` — shared foundation
- **Types.h** — frozen IDs (`Tick`, `HumanId`, `EventId`), `TileCoord` (with canonical
  ordering), `Resource`/`Terrain` enums, event flags (`VERIFIED`, `RUMOR_ONLY`,
  `PLAYER_CAUSED`).
- **Rng.h** — xoshiro256** seeded by splitmix64; 32-byte saveable state; `range`,
  `chance`. The only randomness source in the sim.
- **Fnv.h** — FNV-1a 64-bit accumulator for `stateHash()`.
- **Constants.h**, **Tuning.h** — world size (64×64), default pop (80), tuning skeleton.

### 2.2 `core/dsa/` — the graded library (all own code, no STL containers)
| File | Structure | Key design | Complexity |
|---|---|---|---|
| DynArray.h | dynamic array | doubling capacity, Debug bounds check, movable | push amortised O(1) |
| HashTable.h | hash table | separate chaining via **index links**, dense value array, grow at 0.75, free-list reuse, **deterministic iteration** | avg O(1) |
| MinHeap.h | binary min-heap | key `(priority, seq)` tiebreaker | push/pop O(log n) |
| LinkedList.h | doubly linked list | index-based node pool, O(1) remove by handle | append/remove O(1) |
| Queue.h | circular-buffer FIFO | doubling growth policy | push/pop O(1) |
| AvlTree.h | self-balancing BST | height + rotations, in-order & reverse | insert/erase/find O(log n) |
| Graph.h | directed weighted graph | adjacency lists (out **and** in) for reverse BFS | addEdge O(1) |
| Sorting.h | merge sort + heap sort | comparator fn pointer; merge is stable | O(n log n) |
| DsaStats.h | counters | inserts/probes/collisions/rotations/resizes/comparisons/swaps… | O(1) |

Raw `new[]`/`delete[]` appear **only** inside `DynArray` and `Queue` (our own containers),
as the rules allow. Everything else reuses `DynArray` node pools — no stray allocations.

### 2.3 `core/sim/` — the deterministic engine
- **Command.h** (frozen) — God-Mode commands; the GUI's only mutation path.
- **Scheduler.h** — `MinHeap` of timed events; monotonic `seq` tiebreaker; saveable.
- **ISystem.h / SimContext.h** — pluggable systems; context exposes the four RNG streams,
  world, humans, history, scheduler, due events, metrics.
- **Snapshot.h** (frozen, P1-finalised) — read-only GUI view + `Metrics` + aggregate
  `DsaStats` + recent-event window + `stateHash`.
- **Simulation.h/.cpp** — fixed tick order: **(1)** apply commands → **(2)** pop due
  scheduled events → **(3)** run systems in registration order → **(4)** publish Snapshot
  → **(5)** advance tick. Owns four RNG streams (`world`, `population`, `event`,
  `narration`), world, humans (ascending ID), history, scheduler, command queue.
  `stateHash()` covers tick, seed, all RNG states, every tile, every human (ID order),
  scheduler contents, full history, metrics, pending-command count.
- **HeadlessMain.cpp** — `runner --seed 42 --days 365 --pop 80`; prints population,
  events, hash, narration lines and an obituary.
- **WealthMetrics.h** — Gini coefficient from the AVL wealth index (metrics-only floats).

### 2.4 `core/history/` — history, causation, narrator
- **EventType.h** (25 types), **EventRecord.h** (immutable event).
- **History.h/.cpp** — `record`, `traceRoots` (reverse BFS to root causes), `chain`
  (forward BFS path), `eventsFor` (per-human `LinkedList` for cheap obituaries).
  **Hard rule enforced:** a cause edge is added *only* when a rule passes an explicit
  `causeId`. Cause is never inferred from timing.
- **Grammar.h/.cpp** — ~100-line Tracery-inspired engine: `#symbol#` expansion,
  modifiers `capitalize` / `a`-`an` / plural `s`, recursion guard. **Narration RNG only.**
- **Narrator.h/.cpp** — 25 event types × 3 deadpan variants + obituaries built strictly
  from recorded events. Tone: human behaviour and bad systems only.

### 2.5 `core/persistence/` — save/load
- **SaveLoad.h/.cpp** — `nlohmann/json`, every file carries `"schemaVersion": 1`.
  Saves tick, all four RNG states, tiles, humans (ID order), scheduler heap + `nextSeq`,
  full history, metrics. Schema mismatch is rejected.

### 2.6 Contract placeholders (owned by P2/P3 — clearly marked)
- **core/world/World.h/.cpp** — frozen `World`/`Tile`/`SpatialGrid` API with a
  deterministic placeholder worldgen. `findPath` returns empty (P2 implements Dijkstra/A*).
- **core/humans/Human.h** — frozen plain-data `Human`/`Inventory`/`Explanation`.
- **tools/DemoSystem.h** — DEMO-ONLY life system (kept in `tools/`, never core gameplay)
  so the engine produces real events today. P2/P3 replace it.

### 2.7 Repo root & tools
- **Makefile** (primary verified build, with `-MMD -MP` header dependency tracking),
  **CMakeLists.txt** (cross-platform; MFC GUI guarded behind `if(WIN32)`),
  **CONTRIBUTING.md**, **README.md**, **.gitignore**.
- **tools/SeedHunt.cpp** — scores seeds 1..200 (variety, deaths, scandals, rumor reach,
  corruption discovered), prints top 5, writes `data/scenarios/demo.json`.

---

## 3. Determinism architecture (how it is guaranteed)

1. Single thread; no `rand()`, `std::random_device`, or `std::chrono` in sim logic.
2. Four independent RNG streams; narration never draws from the other three (tested).
3. No `std::unordered_map` iteration in sim logic — our `HashTable` iterates its dense
   array in slot order; humans iterate in ascending ID order.
4. Tiebreakers everywhere: heap `(key, seq)`, events `(tick, id)`, sorted roots ascending.
5. Integer simulation values (attributes 0..100); floats only for metrics/worldgen.
6. Entities reference each other by ID, never pointer.
7. `stateHash()` covers the full canonical state; every determinism test compares hashes.

---

## 4. Verification (REAL output)

### 4.1 Unit tests — Makefile build (clang++ -O2)
```
[doctest] doctest version is "2.4.12"
===============================================================================
[doctest] test cases:      49 |      49 passed | 0 failed | 0 skipped
[doctest] assertions: 2558326 | 2558326 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### 4.2 Unit tests — CMake build (Release, -O3 -DNDEBUG), via CTest
```
Test project /Users/yascine/Desktop/DSA-proj/cmake-build
    Start 1: unit
1/1 Test #1: unit .............................   Passed    0.95 sec
100% tests passed out of 1
```

### 4.3 Randomized DSA tests (100,000 ops each vs the matching std:: container)
- DynArray vs `std::vector`
- HashTable vs `std::unordered_map` (+ identical iteration order across two runs)
- MinHeap vs `std::priority_queue` (min-heap, `(key,seq)`)
- LinkedList vs `std::list`
- Queue vs `std::queue`
- AvlTree vs `std::map` (+ AVL height bound, ascending-insert worst case)
- Graph vs a `std::set` edge model (+ degree consistency, removeNode)
- mergeSort & heapSort vs `std::sort` (+ merge-sort stability)
- Rng vs the **reference C** outputs (seed 1, 1000 draws; plus 6 other seeds)

### 4.4 Determinism — same seed, identical hash (even across optimization levels)
```
Makefile -O2 run1: stateHash: 0x6b30f08a55667c80
Makefile -O2 run2: stateHash: 0x6b30f08a55667c80
CMake -O3 NDEBUG : stateHash: 0x6b30f08a55667c80
```
(seed 42, 365 days, pop 80)

### 4.5 Save/load determinism (tested)
- **200 continuous days == 100 days + save + load + 100 days** → identical hash. ✅
- JSON round-trip preserves hash immediately. ✅
- File save/load round-trip preserves hash (portable temp path). ✅
- Schema mismatch rejected. ✅

### 4.6 Cause graph (tested)
- `Drought → CropFailure → Hunger → Theft`: `traceRoots(theft)` returns `Drought`;
  `chain(Drought, Theft)` returns the full 4-event path. ✅
- Same-tick events with no explicit cause get **no** edge. ✅

### 4.7 Headless runner (seed 42, 365 days, pop 80)
```
seed=42 days=365 initialPop=80
population: 88
events: 2255
births=57 deaths=49 thefts=11 rumors=1073 scandals=28
stateHash: 0x6b30f08a55667c80
```

### 4.8 Seed hunt (seeds 1..200, 180 days, pop 80)
```
rank seed   score variety deaths scandals rumors discovered
 1    144   1097      8      5      35    552       35
 2    182   1030      7      3      34    512       34
 3    124   1024      7      4      32    530       32
 4     96   1012      8     12      26    570       26
 5     32    991      8      8      31    492       31
Wrote data/scenarios/demo.json (seed 144)
```

Full raw logs are saved in `reports/test_output.txt`, `reports/runner_output.txt`,
`reports/seedhunt_output.txt`, `reports/determinism.txt`.

---

## 5. Acceptance checklist (from the guide, §8)

- [x] `core` builds with no GUI includes.
- [x] All DSA structures are own code and pass randomized tests.
- [x] Same seed and commands give the same `stateHash` (two runs).
- [x] Save/load test passes.
- [x] Cause edges come only from explicit rules.
- [x] Diagnostics API returns real counters (`DsaStats` aggregate in Snapshot).
- [x] Narrator and obituaries use only recorded events.
- [x] Every file has a purpose/complexity/users header comment and is explainable.

---

## 6. How to build & run

```
# macOS / Linux (verified)
make test                 # build + run all 49 unit tests
make run                  # headless demo (seed 42, 365 days, pop 80)
./build/seedhunt          # seed hunt → data/scenarios/demo.json

# Cross-platform (verified with CMake 4.5.1)
cmake -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build -j
ctest --test-dir cmake-build --output-on-failure
./cmake-build/runner --seed 144 --days 180 --pop 80
```

---

## 7. Known limitations / hand-off notes

1. **Platform:** this is a Mac. The MFC GUI (P4) is Windows-only and is *not* built here;
   `core` is platform-free and ready for it. CMake guards `gui/` behind `if(WIN32)`.
2. **Placeholders:** `World::generate`/`findPath` and `Human` are contract-faithful
   placeholders. **P2 owns** real worldgen + navigation; **P3 owns** the full human model.
   `tools/DemoSystem.h` is demo-only and must be replaced by P2/P3 systems.
3. **Contracts:** shared headers are frozen. P1 finalised `Snapshot` to use our `DynArray`
   (within P1 ownership). Any further contract change needs the whole team's OK.
4. **Obituary years** are tick/365 year-indices; within a single demo year they read `0-0`.
   This is factual, not a bug; longer runs show distinct years.
5. **Modulo bias** in `Rng::range` is acceptable for a game sim and keeps the port
   byte-identical to the reference usage.

## 8. Suggested next steps for the team
- P2: implement `World::generate` (real ecology) and `findPath` (Dijkstra/A* on our
  MinHeap/Queue/Graph); register an environment system.
- P3: expand `Human`, implement decide/act + society systems; replace `DemoSystem`.
- P4: build the MFC GUI on Windows against `Snapshot` (read) + `Command` (push); wire the
  diagnostics panel to `Snapshot.dsa`.
- Freeze contracts together (Day 2), then integrate headlessly (80 humans, 100 days).
