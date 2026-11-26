// THE GOD EXPERIMENT — Portable Core
// core/Random.hpp — Deterministic RNG foundation (PCG-XSH-RR 32/64 variant).
//
// Algorithm reference: "PCG: A Family of Simple Fast Space-Efficient
// Statistically Good Algorithms for Random Number Generation", Melissa E. O'Neill,
// Harvey Harvey College, 2014. We pin the exact parameterization below so output
// is identical across compilers/platforms (no implementation-defined behavior):
//
//   state' = state * 6364136223846793005 + (inc | 1)          (mod 2^64)
//   xorshifted = ((state >> 18u) ^ state) >> 27u
//   rot  = state >> 59u
//   out  = rotr32(xorshifted, rot)
//
// Default increment: 1442695040888963407u (O'Neill's recommended stream constant).
//
// Stream derivation: each named stream gets its own PCG instance seeded via
// splitmix64(seed, streamSalt) — a documented, fixed mixing function — so
// streams are independent yet fully reproducible from the SeedSet alone.
//
// Save/restore plan (Module 16): persist the raw 64-bit state + increment of
// every active stream; restoring them resumes the sequence bit-exactly.

#pragma once

#include <cstdint>
#include <string_view>
#include <unordered_map>

#include "godexperiment/core/Types.hpp" // SeedSet + std::hash completion order

namespace godex::core {

/// Raw PCG32 generator. Small, header-only, deterministic.
class Pcg32 {
public:
    // Default stream selector. The stored increment is (seq << 1) | 1 = 14427,
    // matching the canonical PCG32 demo default increment.
    static constexpr std::uint64_t kDefaultSeq = 7213u;

    constexpr Pcg32() noexcept : m_inc{(kDefaultSeq << 1u) | 1u} {}
    explicit constexpr Pcg32(std::uint64_t seed, std::uint64_t seq = kDefaultSeq) noexcept
        : m_inc{(seq << 1u) | 1u} {
        m_state = 0u;
        m_state = m_state * kMultiplier + m_inc; // advance once
        m_state += seed;
        m_state = m_state * kMultiplier + m_inc; // advance again
    }

    /// Next 32-bit output. Bit-exact across platforms.
    std::uint32_t nextU32() noexcept {
        const std::uint64_t oldState = m_state;
        m_state = oldState * kMultiplier + m_inc;
        const std::uint32_t word =
            static_cast<std::uint32_t>((oldState >> 18u) | (oldState << 27u)) >> 27u;
        const std::uint32_t xorShifted =
            static_cast<std::uint32_t>(((oldState >> 18u) ^ oldState) >> 27u);
        (void)word;
        const std::uint32_t rot = static_cast<std::uint32_t>(oldState >> 59u);
        return rotr32(xorShifted, rot);
    }

    static constexpr std::uint32_t rotr32(std::uint32_t v, std::uint32_t r) noexcept {
        r &= 31u;
        if (r == 0u) return v;
        return (v >> r) | (v << (32u - r));
    }

    std::uint64_t nextU64() noexcept {
        const std::uint64_t hi = nextU32();
        const std::uint64_t lo = nextU32();
        return (hi << 32u) | lo;
    }

    /// Uniform integer in [lo, hi] via Lemire-style bounded modulo with bias removal (documented so results are portable).
    std::uint32_t nextRange(std::uint32_t lo, std::uint32_t hi) noexcept {
        const std::uint32_t span = hi - lo + 1u;
        if (span == 0u) return lo; // full 32-bit span requested
        const std::uint32_t threshold = (0u - span) % span; // for rejection
        std::uint32_t r;
        do {
            r = nextU32();
        } while (r < threshold);
        return lo + (r % span);
    }

    /// Uniform float in [0, 1) using the top 24 mantissa bits.
    float nextFloat01() noexcept {
        return static_cast<float>(nextU32() >> 8) * (1.0f / 16777216.0f);
    }

    std::uint64_t state() const noexcept { return m_state; }
    std::uint64_t increment() const noexcept { return m_inc; }
    void restore(std::uint64_t state, std::uint64_t inc) noexcept {
        m_state = state;
        m_inc = inc;
    }

private:
    static constexpr std::uint64_t kMultiplier = 6364136223846793005ull;
    std::uint64_t m_state{0};
    std::uint64_t m_inc{1u};
};

/// Fixed, documented splitmix64 used ONLY to derive independent stream seeds.
constexpr std::uint64_t splitmix64(std::uint64_t x) noexcept {
    x += 0x9e3779b97f4a7c15ull;
    x = (x ^ (x >> 30u)) * 0xbf58476d1ce4e5b9ull;
    x = (x ^ (x >> 27u)) * 0x94d049bb133111ebull;
    return x ^ (x >> 31u);
}

/// Named stream registry bound to a SeedSet. Streams are created lazily and
/// deterministically: stream seed = splitmix64(baseSeed ^ salt(streamName)).
class RngStreams {
public:
    explicit RngStreams(const struct SeedSet& seeds) noexcept;

    /// Get (creating on first use) a named stream tied to a category.
    /// Known categories: "world", "population", "event". Additional names
    /// derive from the matching base seed plus a name salt.
    Pcg32& stream(std::string_view name);

    /// Convenience: direct access to the three canonical streams.
    Pcg32& world() { return stream("world"); }
    Pcg32& population() { return stream("population"); }
    Pcg32& events() { return stream("event"); }

private:
    std::uint64_t baseFor(std::string_view name) const noexcept;
    static constexpr std::uint64_t nameSalt(std::string_view name) noexcept {
        std::uint64_t h = 0xcbf29ce484222325ull; // FNV-1a 64
        for (char c : name) {
            h ^= static_cast<std::uint8_t>(c);
            h *= 0x100000001b3ull;
        }
        return h;
    }

    std::uint64_t m_worldSeed;
    std::uint64_t m_populationSeed;
    std::uint64_t m_eventSeed;
    std::unordered_map<std::string, Pcg32> m_streams;
};

} // namespace godex::core
