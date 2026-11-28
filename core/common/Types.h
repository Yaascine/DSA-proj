// Types.h — shared primitive types and enums (FROZEN CONTRACT, P1 owns).
// Purpose: the single source of truth for IDs, coordinates and resource/terrain
//          enums used by every subsystem. Pure C++20, no Windows/MFC includes.
// Complexity: O(1) — plain data only.
// Who uses it: everyone (P1 sim/history, P2 world, P3 humans, P4 gui).
// Rule: signatures here are frozen; changes need the whole team's OK.
#pragma once
#include <cstdint>

namespace tge {

using Tick    = int64_t;   // 1 tick = 1 day
using HumanId = int32_t;   // -1 = none
using EventId = int32_t;   // -1 = none

struct TileCoord {
    int16_t x = 0;
    int16_t y = 0;
    bool operator==(const TileCoord& o) const { return x == o.x && y == o.y; }
    bool operator!=(const TileCoord& o) const { return !(*this == o); }
    // Canonical ordering so any sorted container of coords is deterministic.
    bool operator<(const TileCoord& o) const { return y != o.y ? y < o.y : x < o.x; }
};

enum class Resource : uint8_t { Food = 0, Water = 1, Wood = 2, Stone = 3, COUNT = 4 };
enum class Terrain : uint8_t {
    DeepOcean = 0, Ocean, Lake, Beach, Grass, Forest, Desert, Hills, Mountain, Snow
};

// Event flags (bitmask). A cause edge is only ever added when a rule passes an
// explicit causeId — never inferred from timing.
enum EventFlags : uint32_t {
    EF_NONE          = 0,
    EF_VERIFIED      = 1u << 0,
    EF_RUMOR_ONLY    = 1u << 1,
    EF_PLAYER_CAUSED = 1u << 2
};

} // namespace tge
