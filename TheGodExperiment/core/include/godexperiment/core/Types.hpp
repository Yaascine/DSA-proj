// THE GOD EXPERIMENT — Portable Core
// core/Types.hpp — Fundamental reusable value types.
//
// Design rules:
//   * No <Windows.h>, no MFC, no platform headers here.
//   * Distinct concepts get distinct strong types (EntityId is NOT a tile index).
//   * All arithmetic that can fail is checked and reports failure explicitly.
//
// Module 01 scope only: stable IDs, coordinates, dimensions, tick, seeds,
// normalized values. Human/World/Kingdom classes are documented future work
// (see docs/DATA_CONTRACTS.md) and are intentionally NOT implemented here.

#pragma once

#include <cstdint>
#include <cmath>
#include <limits>
#include <optional>
#include <string>
#include <functional>
#include <unordered_map>

namespace godex::core {

// ---------------------------------------------------------------------------
// EntityId — stable, opaque 64-bit entity identifier.
// Value 0 is reserved for "null / no entity". Ids never recycle within a run;
// save/load must preserve the id space (Module 16).
// ---------------------------------------------------------------------------
class EntityId {
public:
    using underlying = std::uint64_t;

    constexpr EntityId() noexcept : m_value{0} {}
    explicit constexpr EntityId(underlying v) noexcept : m_value{v} {}

    static constexpr EntityId null() noexcept { return EntityId{}; }
    constexpr bool isNull() const noexcept { return m_value == 0; }
    constexpr underlying value() const noexcept { return m_value; }

    friend constexpr bool operator==(const EntityId& a, const EntityId& b) noexcept {
        return a.m_value == b.m_value;
    }
    friend constexpr bool operator!=(const EntityId& a, const EntityId& b) noexcept {
        return !(a == b);
    }
    friend constexpr bool operator<(const EntityId& a, const EntityId& b) noexcept {
        return a.m_value < b.m_value;
    }

    std::string toString() const { return std::to_string(m_value); }

private:
    underlying m_value;
};

// ---------------------------------------------------------------------------
// TileCoord — signed integer grid coordinate. Signed on purpose: worlds may be
// conceptually infinite during generation, and negative intermediate values
// must never wrap around into bogus tiles.
// Convention: X increases rightward, Y increases downward, tile origin top-left.
// ---------------------------------------------------------------------------
struct TileCoord {
    using component = std::int32_t;

    component x{0};
    component y{0};

    constexpr TileCoord() noexcept = default;
    constexpr TileCoord(component px, component py) noexcept : x{px}, y{py} {}

    friend constexpr bool operator==(const TileCoord& a, const TileCoord& b) noexcept {
        return a.x == b.x && a.y == b.y;
    }
    friend constexpr bool operator!=(const TileCoord& a, const TileCoord& b) noexcept {
        return !(a == b);
    }
    // Lexicographic ordering (y-major would be row-major; we use x-major for map keys).
    friend constexpr bool operator<(const TileCoord& a, const TileCoord& b) noexcept {
        return a.x != b.x ? a.x < b.x : a.y < b.y;
    }

    constexpr TileCoord operator+(const TileCoord& o) const noexcept { return {x + o.x, y + o.y}; }
    constexpr TileCoord operator-(const TileCoord& o) const noexcept { return {x - o.x, y - o.y}; }

    // Checked addition — returns nullopt on signed overflow (test requirement).
    static std::optional<TileCoord> tryAdd(const TileCoord& a, const TileCoord& b) noexcept {
        const std::int64_t sx = static_cast<std::int64_t>(a.x) + b.x;
        const std::int64_t sy = static_cast<std::int64_t>(a.y) + b.y;
        if (sx < std::numeric_limits<component>::min() || sx > std::numeric_limits<component>::max() ||
            sy < std::numeric_limits<component>::min() || sy > std::numeric_limits<component>::max()) {
            return std::nullopt;
        }
        return TileCoord{static_cast<component>(sx), static_cast<component>(sy)};
    }

    // Euclidean distance to another tile (double to avoid int sqrt subtleties).
    double distanceTo(const TileCoord& o) const noexcept {
        const double dx = static_cast<double>(x) - o.x;
        const double dy = static_cast<double>(y) - o.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    std::string toString() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
};

// ---------------------------------------------------------------------------
// WorldDimensions — width and height in tiles. Both must be >= 1 and <= limit
// to catch accidental zero/huge values early. Limit is generous (65536 per
// axis) but finite so area stays inside uint64 without wrapping.
// ---------------------------------------------------------------------------
class WorldDimensions {
public:
    static constexpr std::uint32_t kMaxAxis = 65536u;

    constexpr WorldDimensions() noexcept = default;
    constexpr WorldDimensions(std::uint32_t w, std::uint32_t h) noexcept : m_w{w}, m_h{h} {}

    /// True when 1 <= w,h <= kMaxAxis.
    constexpr bool isValid() const noexcept {
        return m_w >= 1u && m_h >= 1u && m_w <= kMaxAxis && m_h <= kMaxAxis;
    }

    constexpr std::uint32_t width() const noexcept { return m_w; }
    constexpr std::uint32_t height() const noexcept { return m_h; }
    constexpr std::uint64_t area() const noexcept {
        return static_cast<std::uint64_t>(m_w) * m_h;
    }

    /// Bounds check for a tile coordinate against this dimension set.
    constexpr bool contains(const TileCoord& c) const noexcept {
        if (c.x < 0 || c.y < 0) return false;
        return static_cast<std::uint32_t>(c.x) < m_w && static_cast<std::uint32_t>(c.y) < m_h;
    }

    friend constexpr bool operator==(const WorldDimensions& a, const WorldDimensions& b) noexcept {
        return a.m_w == b.m_w && a.m_h == b.m_h;
    }

private:
    std::uint32_t m_w{0};
    std::uint32_t m_h{0};
};

// ---------------------------------------------------------------------------
// SimulationTick — discrete simulation time. Signed 64-bit so deltas can be
// negative (rewind tools in God Mode, Module 17) without type tricks.
// One tick currently maps to one in-game hour (final mapping decided in
// Module 05); the type itself is unit-agnostic.
// ---------------------------------------------------------------------------
using SimulationTick = std::int64_t;
inline constexpr SimulationTick kInvalidTick = -1;

// ---------------------------------------------------------------------------
// SeedSet — the three named random streams a world run needs. Kept as plain
// uint64 values; derivation happens in Random.hpp. Persisted verbatim by the
// future save system (Module 16) so runs are exactly reproducible.
// ---------------------------------------------------------------------------
struct SeedSet {
    std::uint64_t worldSeed{0};
    std::uint64_t populationSeed{0};
    std::uint64_t eventSeed{0};

    friend constexpr bool operator==(const SeedSet&, const SeedSet&) noexcept = default;
};

// ---------------------------------------------------------------------------
// NormalizedValue — validated float constrained to [kMin, kMax] == [0, 1].
// Used for attributes like honesty, hunger pressure, temperature weight.
// Construction from float goes through validate()/clamp() so callers choose
// their failure policy explicitly.
// ---------------------------------------------------------------------------
class NormalizedValue {
public:
    static constexpr float kMin = 0.0f;
    static constexpr float kMax = 1.0f;

    constexpr NormalizedValue() noexcept : m_v{kMin} {}
    /// Precondition-checked factory: returns nullopt for NaN or out-of-range.
    static constexpr std::optional<NormalizedValue> validate(float v) noexcept {
        if (std::isnan(v) || v < kMin || v > kMax) return std::nullopt;
        NormalizedValue n;
        n.m_v = v;
        return n;
    }
    /// Saturating conversion (use at data-ingestion boundaries only).
    static constexpr NormalizedValue clamp(float v) noexcept {
        if (std::isnan(v)) return NormalizedValue{};
        NormalizedValue n;
        n.m_v = v < kMin ? kMin : (v > kMax ? kMax : v);
        return n;
    }
    /// Explicit trusted constructor (values coming from internal math).
    struct TrustedTag {};
    constexpr NormalizedValue(float v, TrustedTag) noexcept : m_v{v} {}

    constexpr float value() const noexcept { return m_v; }
    /// Map to an integer range, e.g. percentage display: toInt(0, 100).
    constexpr int toInt(int lo, int hi) const noexcept {
        return lo + static_cast<int>(m_v * (hi - lo) + 0.5f);
    }

    friend constexpr bool operator==(const NormalizedValue& a, const NormalizedValue& b) noexcept {
        return a.m_v == b.m_v;
    }

private:
    float m_v;
};

} // namespace godex::core

// Hash support for use as unordered_map keys later. Declared via the standard
// specialization macro so <functional> is guaranteed complete at this point.
namespace std {
template <>
struct hash<godex::core::EntityId> {
    std::size_t operator()(const godex::core::EntityId& id) const noexcept {
        return std::hash<std::uint64_t>{}(id.value());
    }
};
template <>
struct hash<godex::core::TileCoord> {
    std::size_t operator()(const godex::core::TileCoord& c) const noexcept {
        // Simple mix of two int32s into one size_t.
        return (static_cast<std::size_t>(static_cast<std::uint32_t>(c.x)) << 32) |
               static_cast<std::uint32_t>(c.y);
    }
};
} // namespace std
