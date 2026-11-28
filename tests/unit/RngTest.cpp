// RngTest.cpp — verify the C++ xoshiro256**/splitmix64 port against the reference C.
#include "doctest.h"
#include "core/common/Rng.h"
#include "../reference/prng_ref.h"

TEST_CASE("Rng matches reference C outputs for seed 1 (1000 draws)") {
    tge::Rng r(1);
    ref_seed(1);
    for (int i = 0; i < 1000; ++i) {
        uint64_t got = r.next();
        uint64_t want = ref_next();
        CHECK(got == want);
    }
}

TEST_CASE("Rng matches reference for several seeds") {
    const uint64_t seeds[] = {0, 2, 42, 12345, 0xDEADBEEFULL, 0xFFFFFFFFFFFFFFFFULL};
    for (uint64_t sd : seeds) {
        tge::Rng r(sd);
        ref_seed(sd);
        for (int i = 0; i < 200; ++i) CHECK(r.next() == ref_next());
    }
}

TEST_CASE("Rng is reproducible and streams are independent") {
    tge::Rng a(42), b(42);
    for (int i = 0; i < 100; ++i) CHECK(a.next() == b.next());
    tge::Rng c(42), d(43);
    bool differ = false;
    for (int i = 0; i < 10; ++i) if (c.next() != d.next()) differ = true;
    CHECK(differ);
}

TEST_CASE("Rng range and chance stay in bounds") {
    tge::Rng r(7);
    for (int i = 0; i < 10000; ++i) {
        int v = r.range(5, 9);
        CHECK(v >= 5);
        CHECK(v <= 9);
        int p = r.range(1, 100);
        CHECK(p >= 1);
        CHECK(p <= 100);
    }
}
