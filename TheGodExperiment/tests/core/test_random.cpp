// THE GOD EXPERIMENT — Tests
// Test 02: deterministic randomness (PCG32 + stream derivation).
#include "TestFramework.hpp"
#include "godexperiment/core/Random.hpp"
#include "godexperiment/core/Types.hpp"

using namespace godex::core;

TEST_CASE(Pcg_same_seed_same_sequence) {
    Pcg32 a{12345}, b{12345};
    for (int i = 0; i < 1000; ++i) CHECK_EQ(a.nextU32(), b.nextU32());
}

TEST_CASE(Pcg_different_seeds_differ) {
    Pcg32 a{1}, b{2};
    bool anyDiff = false;
    for (int i = 0; i < 16; ++i)
        if (a.nextU32() != b.nextU32()) anyDiff = true;
    CHECK(anyDiff);
}

TEST_CASE(Pcg_known_test_vectors) {
    // (a) Canonical PCG reference vectors — PCG-XSH-RR 32/64, init(seed=42,
    //     seq=54). These match the published pcg_reference test output, which
    //     validates our algorithm implementation against an external source.
    Pcg32 ref{42, 54};
    const std::uint32_t expectedRef[8] = {
        0xa15c02b7u, 0x7b47f409u, 0xba1d3330u, 0x83d2f293u,
        0xbfa4784bu, 0xcbed606eu, 0xbfc6a3adu, 0x812fff6du};
    for (int i = 0; i < 8; ++i) CHECK_EQ(ref.nextU32(), expectedRef[i]);

    // (b) Frozen vectors for our project default stream
    //     (init(seq=7213) => stored increment 14427), seed 42.
    Pcg32 g{42};
    const std::uint32_t expectedDefault[8] = {
        0xdf7b60ffu, 0x1dbaccb6u, 0xc8e6e3a3u, 0x63ae9b8fu,
        0x7e1125d4u, 0x2f4536c8u, 0x657994a6u, 0xa3720090u};
    for (int i = 0; i < 8; ++i) CHECK_EQ(g.nextU32(), expectedDefault[i]);
}

TEST_CASE(Pcg_state_restore) {
    Pcg32 a{999};
    a.nextU32(); a.nextU32();
    const auto s = a.state(), inc = a.increment();
    const std::uint32_t nextA = a.nextU32();
    Pcg32 b;
    b.restore(s, inc);
    CHECK_EQ(b.nextU32(), nextA);
}

TEST_CASE(Pcg_range_bounds) {
    Pcg32 g{7};
    for (int i = 0; i < 5000; ++i) {
        const auto v = g.nextRange(3, 9);
        if (v < 3 || v > 9) { CHECK(false); break; }
    }
    CHECK(true);
    // degenerate single-value range
    CHECK_EQ(g.nextRange(5, 5), 5u);
    // full span request must not crash / loop
    volatile auto f = g.nextRange(0, 0xFFFFFFFFu);
    (void)f;
    CHECK(true);
}

TEST_CASE(Pcg_float_domain) {
    Pcg32 g{31337};
    for (int i = 0; i < 2000; ++i) {
        const float f = g.nextFloat01();
        if (!(f >= 0.0f && f < 1.0f)) { CHECK(false); break; }
    }
    CHECK(true);
}

TEST_CASE(Streams_independent_and_reproducible) {
    SeedSet seeds{11, 22, 33};
    RngStreams s1{seeds};
    RngStreams s2{seeds};
    // Same seed set => identical streams.
    CHECK_EQ(s1.world().nextU32(), s2.world().nextU32());
    CHECK_EQ(s1.population().nextU32(), s2.population().nextU32());
    CHECK_EQ(s1.events().nextU32(), s2.events().nextU32());
    // Named streams differ from each other.
    RngStreams s3{seeds};
    const auto w = s3.world().nextU32();
    const auto p = s3.population().nextU32();
    const auto e = s3.events().nextU32();
    CHECK(w != p);
    CHECK(p != e);
    // Auxiliary named stream derived deterministically too.
    RngStreams s4{seeds}, s5{seeds};
    CHECK_EQ(s4.stream("weather").nextU32(), s5.stream("weather").nextU32());
    CHECK(s4.stream("weather").nextU32() != s4.world().nextU32() ||
          s4.stream("weather").nextU32() != s4.world().nextU32());
}

TEST_CASE(Splitmix64_fixed_vector) {
    // Independent Python oracle (see reports) confirms these values:
    CHECK_EQ(splitmix64(0ull), 0xe220a8397b1dcdafull);
    CHECK_EQ(splitmix64(1ull), 0x910a2dec89025cc1ull);
}

int main() { return testfw::runAll("randomness"); }
