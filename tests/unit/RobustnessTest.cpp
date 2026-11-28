// RobustnessTest.cpp — hash-table shrink and RNG input guards.
#include "doctest.h"
#include "core/dsa/HashTable.h"
#include "core/common/Rng.h"

TEST_CASE("HashTable shrinks after bulk erase") {
    tge::HashTable<int> h;
    for (int i = 0; i < 2000; ++i) h.put(i, i);
    size_t big = h.bucketCount();
    for (int i = 0; i < 1990; ++i) h.erase(i);
    CHECK(h.size() == 10);
    CHECK(h.bucketCount() < big);
    CHECK(h.bucketCount() >= 8);
    for (int i = 1990; i < 2000; ++i) { int g = 0; CHECK(h.get(i, g)); CHECK(g == i); }
}

TEST_CASE("Rng range and chance guard bad inputs without crashing") {
    tge::Rng r(1);
    CHECK(r.range(5, 5) == 5);
    CHECK(r.range(9, 2) == 9);   // reversed range -> lo, no division by zero
    CHECK_FALSE(r.chance(0));
    CHECK(r.chance(100));
    CHECK_FALSE(r.chance(-5));   // clamped to 0
    CHECK(r.chance(150));        // clamped to 100
}
