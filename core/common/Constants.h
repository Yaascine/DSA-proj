// Constants.h — fixed world / simulation constants (shared facts).
// Purpose: one place for the numbers every subsystem agrees on.
// Complexity: O(1).
// Who uses it: everyone.
#pragma once
#include <cstdint>

namespace tge {

constexpr int WORLD_W = 64;
constexpr int WORLD_H = 64;
constexpr int DEFAULT_POPULATION = 80;
constexpr int TICKS_PER_YEAR = 365;

// Snapshot keeps only the last N events for the GUI ticker.
constexpr int SNAPSHOT_EVENT_WINDOW = 64;

// Attribute values are integers 0..100 unless noted otherwise.
constexpr int ATTR_MIN = 0;
constexpr int ATTR_MAX = 100;

} // namespace tge
