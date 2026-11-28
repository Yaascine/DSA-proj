# AGENTS.md — rules for AI agents working on The God Experiment

These rules are mandatory. They protect determinism, the graded DSA code, and the
team's shared contracts. Read the relevant `P*_...md` guide before editing.

## Hard rules
1. **Contracts first.** Shared headers (`core/common/Types.h`, `core/world/World.h`,
   `core/humans/Human.h`, `core/history/History.h`, `core/sim/Command.h`,
   `core/sim/Snapshot.h`, `core/sim/ISystem.h`) are FROZEN. Do not change a signature
   without the whole team's OK. Never "improve" a contract quietly.
2. **Small tasks.** One concept per file. Never ask an agent for a "whole phase".
3. **Every task ends with a passing test and its REAL pasted output.** Run the test
   yourself; never trust "tests pass" without seeing the output.
4. **Plain, commented C++.** Header comment on every file: purpose, complexity, who
   uses it. No clever templates. If you cannot explain a file, reject it.
5. **One person merges.** Do not push to shared branches or rewrite others' work.

## Determinism (do not break)
- Single thread. No `rand()`, no `std::random_device`, no `std::chrono` in sim logic.
- Use `core/common/Rng.h` only. Four streams: `world`, `population`, `event`,
  `narration`. Narration must never draw from the other three.
- Never iterate `std::unordered_map` in simulation logic. Iterate in ascending ID
  order or over our own hash table's dense array.
- Every ordering needs a tiebreaker: events `(tick, seq)`; heap ties by `seq`.
- Simulation values are integers (attributes 0..100). Floats only for metrics and
  one-time world generation.
- Entities refer to each other by ID, never by pointer.
- `stateHash()` covers the full canonical state; determinism tests compare hashes.

## Architecture
- `core` must NOT include `<windows.h>` or any MFC header.
- The GUI never mutates simulation state; it pushes `Command`s and reads `Snapshot`.
- The graded DSA structures in `core/dsa/` are our OWN code — no STL containers for
  them, no copying from reference repos. Each carries `DsaStats` counters.
- No raw `new`/`delete` outside our own containers.

## Build & test
- macOS/Linux: `make test` (clang++ C++20). Cross-platform: CMake (`cmake -B build`).
- The MFC GUI (`gui/`, P4) is Windows-only and is not built on macOS.
