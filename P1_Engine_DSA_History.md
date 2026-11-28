# P1 GUIDE: Engine, DSA Library, History and Narrator

Project: **The God Experiment** (DSA course, MFC required, presentation in about 2 weeks)
Your role: the backbone. Everyone builds on your code, so your pieces ship first.
Last research check: 2026-11-28 (web search). Pin library versions yourself; see section 6.

---

## 0. Shared project facts (identical in all four guides)

- World: 64x64 tiles, about 80 humans, 1 tick = 1 day, fixed seeds, Windows, MFC GUI drawn with GDI.
- Core is pure C++20 with NO Windows or MFC includes. The GUI only reads a Snapshot and sends Commands.
- Folder layout:

```
TheGodExperiment/
  AGENTS.md  README.md  CMakeLists.txt  TheGodExperiment.sln
  core/
    common/ dsa/ sim/ history/ persistence/        <- P1
    world/ environment/ navigation/ experiments/   <- P2
    humans/ society/                               <- P3
  gui/                                             <- P4 (MFC)
  tests/  data/scenarios/  data/populations/  reports/  docs/
```

- Shared contracts, FROZEN on Day 2 (signatures are sketches; the owner finalizes, then nobody changes them without the whole team's OK):

```cpp
// core/common/Types.h  (P1)
using Tick = int64_t;  using HumanId = int32_t;  using EventId = int32_t;
struct TileCoord { int16_t x, y; };
enum class Resource : uint8_t { Food, Water, Wood, Stone, COUNT };
enum class Terrain  : uint8_t { DeepOcean, Ocean, Lake, Beach, Grass, Forest, Desert, Hills, Mountain, Snow };

// core/world/World.h  (P2 implements; P3 and P4 call)
struct Tile { Terrain terrain; uint8_t elevation, temperature, moisture, fertility, vegetation; // 0..100
              uint16_t stock[4];   // per Resource
              uint8_t fire;        // 0 = none, >0 = burning ticks left
              uint8_t flood;       // 0..100 water depth
              int16_t owner; };    // settlement id or -1
class World {
  int width() const; int height() const;
  const Tile& tile(TileCoord) const;
  bool passable(TileCoord) const;          // false for deep water, active fire, deep flood
  int  moveCost(TileCoord) const;
  int  takeResource(TileCoord, Resource, int amount);   // returns what was REALLY taken; never creates
  void addResource (TileCoord, Resource, int amount);   // only regrowth rules and God Mode call this
  std::vector<TileCoord> findPath(TileCoord from, TileCoord to) const;
};
class SpatialGrid { void move(HumanId, TileCoord from, TileCoord to);
                    void nearby(TileCoord, int radius, std::vector<HumanId>& out) const; };

// core/humans/Human.h  (P3) - plain data, integers 0..100 unless noted
struct Human { HumanId id; std::string name; int ageDays; bool alive; TileCoord pos;
               /* attributes: see P3 guide */ Inventory inv; ActionType action; /* ... */ };
struct Explanation { ActionType chosen; struct Line{ const char* what; int value, weight, contribution; };
                     Line lines[8]; int lineCount; };

// core/history/History.h  (P1)
EventId History::record(EventType, Tick, std::vector<HumanId> actors, TileCoord where,
                        int param, std::vector<EventId> causes, uint32_t flags);

// core/sim/Command.h  (P1 queue; P4 pushes; P2/P3 handlers apply)
enum class CmdType { SpawnHuman, EditAttribute, AddResource, StartFire, StartFlood, PlantRumor, SetLeader };
struct Command { CmdType type; int a, b, c, d; HumanId target; };

// core/sim/Snapshot.h  (P1 builds; P4 reads, read-only)
struct Snapshot { Tick tick; const World* world; const std::vector<Human>* humans;
                  /* counters, last N events, DSA stats */ };

// core/sim/ISystem.h  (P1)
struct ISystem { virtual void onTick(SimContext&) = 0; virtual ~ISystem() {} };
```

- Tick order (fixed, P1 enforces): 1) apply commands, 2) pop due scheduled events from the heap, 3) environment (P2), 4) humans decide and act in ascending ID order (P3), 5) resolve interactions (theft, rumors, births), 6) record events and metrics, 7) publish Snapshot.
- Rules for working with Claude Code: contracts first, small tasks, every task ends with a passing test and its REAL pasted output, plain commented C++ with one concept per file, one person merges, never ask an agent for a "whole phase".

---

## 1. What you own

| Folder | Content |
|---|---|
| `core/common/` | Types, Rng, constants, Tuning.h skeleton |
| `core/dsa/` | Your own DSA library (graded) |
| `core/sim/` | Simulation loop, command queue, scheduler, Snapshot, state hash, headless runner |
| `core/history/` | Event log, cause graph, narrator, obituaries |
| `core/persistence/` | Save/load JSON, determinism tests |
| repo root | CMake, doctest setup, `AGENTS.md`, merging |

---

## 2. Strict parameters (do not break these)

**Determinism**
1. Single thread. No `rand()`, no `std::random_device`, no `std::chrono` inside simulation logic.
2. Use your own RNG (section 4.1). Four separate streams: `world`, `population`, `event`, `narration`. Narration text must never consume numbers from the other three, or editing a joke would change history.
3. Never iterate `std::unordered_map` in simulation logic. Iterate in ascending ID order or over your own hash table's dense array.
4. Every ordering needs a tiebreaker: events sort by `(tick, seq)`; heap ties break by `seq`.
5. Store simulation values as integers (attributes 0..100). Floats may be used only for metrics and for one-time world generation (P2).
6. Entities refer to each other by ID, never by pointer.

**Architecture**
7. `core` must not include `<windows.h>` or any MFC header.
8. The GUI never mutates simulation state. It pushes `Command`s.
9. `stateHash()` covers the full canonical state, and every determinism test compares hashes.

**Code style (for the Q&A and the walkthrough)**
10. One concept per file. Header comment on every file: purpose, complexity, who uses it.
11. No clever templates. Simple structs and plain classes. Templates only where a container truly needs it (keep them readable).
12. No raw `new`/`delete` outside your own containers.
13. Each structure carries operation counters (`DsaStats`) so the diagnostics panel shows real numbers.

---

## 3. Deliverables

### 3.1 DSA library (all your own code, no STL for these)

| File | Structure | Design choice | Used for |
|---|---|---|---|
| `DynArray.h` | Dynamic array | Doubles capacity; bounds check in Debug | Entity storage |
| `HashTable.h` | Hash table | Separate chaining via index links; grows at load factor 0.75; values live in a dense array so iteration order is deterministic | ID to human/settlement lookup |
| `MinHeap.h` | Binary min-heap | Array-based; key is `(priority, seq)` | Event scheduler, Dijkstra (P2) |
| `LinkedList.h` | Doubly linked list | O(1) append/remove | Chronological history log, rumor path logs |
| `Queue.h` | Circular-buffer queue | Fixed growth policy | BFS (rumors, evacuation) |
| `AvlTree.h` | Self-balancing BST | Insert, erase, in-order and reverse in-order | Wealth index: richest N, Gini |
| `Graph.h` | Directed weighted graph, adjacency lists | add/remove node and edge, neighbor iteration | Relationships (P3), cause graph (P1) |
| `Sorting.h` | Merge sort and heap sort | Comparator function | Rankings, reports |
| `DsaStats.h` | Counters | inserts, probes, collisions, rotations, resizes, comparisons, swaps | Diagnostics panel (P4) |

P2 builds BFS, Dijkstra and A* on top of your heap, queue and graph. You write only the traversals your history needs (reverse BFS/DFS on the cause graph).

**Test trick (cheap and impressive):** for every structure, run 100,000 random operations against the matching `std::` container and assert identical results. Put it in `tests/unit/`.

### 3.2 Simulation loop
- `Simulation` owns the systems in a fixed order, the command queue (your `Queue`), the scheduler (your `MinHeap` of timed events) and the four RNG streams.
- API: `step()`, `run(n)`, `pushCommand(cmd)`, `stateHash()`, `snapshot()`.
- Headless runner `core/sim/HeadlessMain.cpp`: console program, `runner --seed 42 --days 365 --pop 80` prints population, events and the hash. This is how you prove everything works without the GUI.

### 3.3 History and cause graph
- `EventRecord { id, tick, type, actors[], where, param, causeIds[], flags }`.
- Flags: `VERIFIED`, `RUMOR_ONLY`, `PLAYER_CAUSED`.
- Hard rule: **a cause edge is added only when a rule passes a `causeId` explicitly** (a theft event cites the hunger or famine event that triggered it). Never infer cause from timing.
- Cause graph = your `Graph`, edge from cause to effect.
- API: `record(...)`, `traceRoots(eventId)` (reverse BFS to events with no causes), `chain(from, to)`, `eventsFor(humanId)` (hash table from HumanId to a linked list of EventIds, so obituaries are cheap).

### 3.4 Narrator and obituaries
- A tiny template engine you write yourself (about 100 lines), inspired by Tracery: a map from symbol name to a list of strings, expansion of `#symbol#`, and three modifiers (`capitalize`, `a/an`, plural `s`). Variants are picked with the `narration` RNG stream.
- Tone: deadpan nature-documentary or obituary voice. The jokes are about human behavior and bad systems only. Never real groups, religions or nationalities. Never sexual violence (the model contains none).
- Budget real time for writing lines: about 25 event types, 3 variants each. Ask the others for lines too.

Starter lines (replace with your own, keep them factual to the event data):

```
Theft:        "#actor# took #n# #item# from #victim#. #actor# calls it redistribution. #victim# calls it theft."
              "Day #tick#: #victim# is down #n# #item#. #actor# is up #n# #item# and down one friend."
Hoarding:     "#actor# is sitting on #n# units of food during the famine. Bold strategy."
Gossip:       "#speaker# told #listener# about #subject#. Accuracy: #accuracy#."
Embezzle:     "The storehouse is short #n# food. #leader# has never looked healthier."
Discovered:   "#leader# was caught skimming the storehouse. The ledger had opinions."
Leader:       "#name# became leader. #votes# people voted. #names# were not consulted."
Fire:         "A fire started near #place#. It has no plan, but it is executing it well."
Rescue:       "#actor# carried #victim# out of the flood. #victim# will not be asked about the bread incident."
Death:        "#name# died of #cause# at #age#. #survivors# survive them. #grudges# grudges survive them too."
```

Obituary (built only from recorded events, never invented):
```
#name# (#birthYear#-#deathYear#), #occupation#. Survived by #children# children and #grudges# grudges.
Cause of death: #cause#. Best known for: #topEvent#. Rumored last words: "#rumorQuote#" (unverified).
```

### 3.5 Save/load
- `nlohmann/json`. Every file has `"schemaVersion"`.
- Save: tick, all four RNG states, tiles, humans (ID order), scheduler heap contents, history.
- Required tests:
  - 200 continuous days produce the same `stateHash` as 100 days, save, load, 100 days.
  - Same seed and same command list, run twice, give the same hash.

### 3.6 Seed hunting tool (days 12-13)
`tools/SeedHunt.cpp`: run seeds 1..200 headless for 180 days, score each by event variety, deaths, scandals, rumor reach and corruption discovered, and print the top 5. The team picks one for the demo and records it in `data/scenarios/demo.json`. This is legitimate: all events are real simulation output.

---

## 4. Code chunks you can use

### 4.1 RNG (public-domain algorithms by Blackman and Vigna)
Reference files: `prng.di.unimi.it/xoshiro256starstar.c` and `splitmix64.c`. **Test your port against the outputs of the reference C files.**

```cpp
// core/common/Rng.h : xoshiro256** seeded by splitmix64. Deterministic, 32 bytes of state.
#include <cstdint>
struct Rng {
    uint64_t s[4];
    static uint64_t splitmix64(uint64_t& x) {
        uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }
    explicit Rng(uint64_t seed = 1) { for (auto& v : s) v = splitmix64(seed); }
    static uint64_t rotl(uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }
    uint64_t next() {
        uint64_t result = rotl(s[1] * 5, 7) * 9;
        uint64_t t = s[1] << 17;
        s[2] ^= s[0]; s[3] ^= s[1]; s[1] ^= s[2]; s[0] ^= s[3];
        s[2] ^= t;    s[3] = rotl(s[3], 45);
        return result;
    }
    int range(int lo, int hi) { return lo + (int)(next() % (uint64_t)(hi - lo + 1)); }  // inclusive
    bool chance(int percent)  { return range(1, 100) <= percent; }
};
```
Separate streams: `Rng world(seedWorld), population(seedPop), event(seedEvent), narration(seedEvent ^ 0xABCDEF)`.

### 4.2 State hash (FNV-1a 64-bit)
```cpp
struct Fnv { uint64_t h = 14695981039346656037ULL;
             void add(uint8_t b) { h ^= b; h *= 1099511628211ULL; }
             void add64(uint64_t v) { for (int i = 0; i < 8; ++i) add((uint8_t)(v >> (8 * i))); } };
```
Feed it every canonical field in ID order. If two runs differ, bisect by hashing per system.

### 4.3 Min-heap (non-template, readable)
```cpp
struct HeapItem { int64_t key; int32_t seq; int32_t payload; };   // order by key, then seq
class MinHeap {
    DynArray<HeapItem> a; DsaStats* st;
    static bool less(const HeapItem& x, const HeapItem& y) { return x.key < y.key || (x.key == y.key && x.seq < y.seq); }
    void siftUp(size_t i) { while (i > 0) { size_t p = (i - 1) / 2; st->comparisons++;
                              if (!less(a[i], a[p])) break; swap(a[i], a[p]); st->swaps++; i = p; } }
    void siftDown(size_t i) { size_t n = a.size();
        for (;;) { size_t l = 2*i+1, r = l+1, m = i;
            if (l < n && less(a[l], a[m])) m = l;
            if (r < n && less(a[r], a[m])) m = r;
            if (m == i) break; swap(a[i], a[m]); st->swaps++; i = m; } }
  public: void push(HeapItem it); HeapItem pop(); const HeapItem& peek() const; bool empty() const;
};
```

### 4.4 Narrator grammar
```cpp
// core/history/Grammar.h
class Grammar {
    std::map<std::string, std::vector<std::string>> rules;   // narrator only, never used for simulation
  public:
    void add(const std::string& symbol, const std::string& text);
    std::string expand(const std::string& symbol, const Vars& vars, Rng& narration) const;
    // expand(): pick a variant with narration RNG, replace each #name# from vars or recursively from rules,
    // apply optional modifiers like #name.capitalize#
};
```

### 4.5 Gini coefficient from your AVL (metrics only, floats are fine here)
Walk the AVL in ascending wealth order into an array `x[0..n-1]`, then
`G = (2 * sum(i * x[i-1]) for i=1..n) / (n * sum(x)) - (n + 1) / n`. Test with all-equal wealth (G = 0) and one person owning everything (G close to 1).

---

## 5. Claude Code task prompts (copy and edit)

1. "Implement `core/dsa/MinHeap.h` by hand (no STL containers, no `std::priority_queue`): push, pop, peek, size, empty, ordered by `(key, seq)`. Count comparisons and swaps in `DsaStats`. Comment the file with purpose and big-O. Write `tests/unit/MinHeapTest.cpp` with doctest comparing 100,000 random operations to `std::priority_queue`. Run the tests and paste the real output."
2. "Implement `core/common/Rng.h` (xoshiro256** seeded by splitmix64) and test it against the reference C code outputs for seed 1."
3. "Implement `HashTable.h`: separate chaining via index links, dense value array, resize at load factor 0.75, collision and probe counters. Test against `std::unordered_map` with 100,000 random ops, then check iteration order is identical across two runs."
4. "Implement `Simulation` with fixed system order, command queue, scheduler heap and `stateHash()`. Add the headless runner. Show two runs with seed 42 printing the same hash."
5. "Implement `History` with `record`, `traceRoots`, `chain`, `eventsFor`. Add a test where Drought -> CropFailure -> Hunger -> Theft is recorded and `traceRoots(theft)` returns Drought."

---

## 6. Open-source references (checked via web search on 2026-11-28)

| Item | Status | What to take |
|---|---|---|
| **nlohmann/json** (github.com/nlohmann/json) | Verified, MIT (`LICENSE.MIT`), single header `single_include/nlohmann/json.hpp` | Save files. Pattern: `json j = json::parse(std::ifstream("f.json"));` and `std::ofstream o("f.json"); o << std::setw(4) << j;` |
| **doctest** (github.com/doctest/doctest) | Verified, MIT, single header (distro packages list 2.4.12 and 2.5.2; pin whichever tag you download) | Tests. One file defines `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN` before `#include "doctest.h"`; others just include it and write `TEST_CASE(...)` with `CHECK(...)` |
| **xoshiro256** and splitmix64** (prng.di.unimi.it) | Verified, reference C files published by the authors | RNG algorithms (chunk 4.1). Port and test against the reference output |
| **Tracery** (github.com/galaxykate/tracery) | Verified: JavaScript, Apache-2.0; also `tracery-ts` | Idea only: `#symbol#` expansion with modifiers. You write your own C++ version |
| Caves of Qud: "Subverting Historical Cause & Effect: Generation of Mythic Biographies" | Title seen in search results, not read in full | Inspiration for biography generation from recorded events |
| **Ichurri/Data-Structures** (MIT), **vera-codes6/c-cpp-algorithms-implementation** (MIT, C++17), **Welding-Torch/Algorithms-And-Data-Structures** | Verified: contain hash table with chaining, AVL, min-heap/heap sort, queues | Compare your structures against them. Do NOT copy: the graded structures must be yours |
| TheAlgorithms/C-Plus-Plus | Not checked this session | Another comparison source if you want one |

**Repos from your original list that I could not find or confirm:** `djeyprott/sim` and `Entendore/CivSim`. Search returned unrelated projects (for example `Crynge/CivSim`, a Python/TypeScript framework, is a different project). Do not plan around them unless you paste the real links.

**Reality check:** none of the verified simulation repos is C++, so for the engine you are writing code, not importing it.

---

## 7. Your schedule

- **Days 1-2:** repo, solution, CMake, doctest, `AGENTS.md`. Freeze shared headers with the team. `Rng`, `DynArray`, `HashTable` with tests.
- **Day 3:** `MinHeap`, `Queue`, `LinkedList`.
- **Day 4:** `AvlTree`, `Graph`, `Sorting`, `DsaStats`.
- **Day 5:** simulation loop, scheduler, commands, `stateHash`, headless runner.
- **Day 6:** history and cause graph.
- **Day 7:** first integration with P2 and P3 (80 humans for 100 days headless).
- **Days 8-9:** narrator, obituaries, save/load.
- **Day 10:** determinism and save/load tests; diagnostics API for P4.
- **Day 11:** feature freeze.
- **Days 12-13:** seed hunting, Q&A prep, bug fixing.
- **Day 14:** rehearsal and backup video.

---

## 8. Acceptance checklist

- [ ] `core` builds with no GUI includes.
- [ ] All DSA structures are your own code and pass the randomized tests.
- [ ] Same seed and commands give the same `stateHash` (two runs).
- [ ] Save/load test passes.
- [ ] Cause edges come only from explicit rules.
- [ ] Diagnostics API returns real counters.
- [ ] Narrator and obituaries use only recorded events.
- [ ] You can open any file you wrote and explain it.

## 9. Likely questions you will be asked

- Why a hash table with a dense array instead of `std::unordered_map`?
- Complexity of heap push and pop? Why do ties need `seq`?
- Why AVL instead of a plain BST?
- How do you know the simulation is deterministic?
- How does the cause graph avoid false causation?
- Where is the `Human` class? (Point to P3's `core/humans/Human.h` and know its fields.)

## 10. Pitfalls

- A missing tiebreaker makes hashes differ for no visible reason. Add one everywhere.
- Do not let agents "improve" the contracts quietly. Contract changes need the whole team.
- Agents love over-engineering. If a file uses clever templates you cannot explain, reject it.
- Run every test yourself and look at the output. Never trust "tests pass" without output.
