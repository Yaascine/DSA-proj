// SaveLoad.h — JSON save/load of the FULL canonical state (P1).
// Purpose: persist tick, all four RNG states, tiles, humans (ID order), scheduler
//          heap contents, history and metrics. Every file carries "schemaVersion".
// Library: nlohmann/json v3.11.3 (MIT, single header).
// Determinism guarantee: 200 continuous days == 100 days + save + load + 100 days
//                          (identical stateHash). Verified in tests.
// Complexity: O(tiles + humans + events).
// Who uses it: P1 persistence/tests; P4 gui save/load buttons.
#pragma once
#include <string>
#include "core/sim/Simulation.h"

namespace tge {
namespace SaveLoad {

constexpr int SCHEMA_VERSION = 1;

std::string toJson(const Simulation& sim);
// Restore into an existing Simulation (construct it with the saved seed and 0 pop,
// re-register the same systems, then call this). Returns false on schema mismatch.
bool fromJson(Simulation& sim, const std::string& text);

bool save(const Simulation& sim, const std::string& path);
bool loadInto(Simulation& sim, const std::string& path);

} // namespace SaveLoad
} // namespace tge
