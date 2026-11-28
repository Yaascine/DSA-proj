// Rng.h — deterministic PRNG: xoshiro256** seeded by splitmix64.
// Purpose: the ONLY randomness source in the simulation. Public-domain algorithms
//          by Blackman & Vigna (prng.di.unimi.it). 32 bytes of state, saveable.
// Complexity: next() O(1); seeding O(1) (4 splitmix64 rounds).
// Who uses it: P1 sim (four streams: world, population, event, narration).
// Determinism rules: no rand(), no std::random_device, no std::chrono in sim logic.
//   The narration stream must NEVER draw from the other three, so editing a joke
//   cannot change history. Test the port against the reference C outputs.
#pragma once
#include <cstdint>

namespace tge {

struct Rng {
    uint64_t s[4];

    // splitmix64: used only to expand a single seed into the 4-word state.
    static uint64_t splitmix64(uint64_t& x) {
        uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    explicit Rng(uint64_t seed = 1) {
        uint64_t x = seed;
        for (auto& v : s) v = splitmix64(x);
    }

    static uint64_t rotl(uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }

    // xoshiro256** next(). Matches the reference C exactly.
    uint64_t next() {
        const uint64_t result = rotl(s[1] * 5, 7) * 9;
        const uint64_t t = s[1] << 17;
        s[2] ^= s[0];
        s[3] ^= s[1];
        s[1] ^= s[2];
        s[0] ^= s[3];
        s[2] ^= t;
        s[3] = rotl(s[3], 45);
        return result;
    }

    // Inclusive integer range [lo, hi]. Modulo bias is acceptable for a game sim
    // and keeps the port byte-identical to the simple reference usage.
    int range(int lo, int hi) { return lo + (int)(next() % (uint64_t)(hi - lo + 1)); }
    bool chance(int percent) { return range(1, 100) <= percent; }

    bool operator==(const Rng& o) const {
        return s[0] == o.s[0] && s[1] == o.s[1] && s[2] == o.s[2] && s[3] == o.s[3];
    }
};

} // namespace tge
