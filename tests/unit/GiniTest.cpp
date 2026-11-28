// GiniTest.cpp — inequality metric from the AVL wealth index (floats OK for metrics).
#include "doctest.h"
#include "core/sim/WealthMetrics.h"
#include <cmath>

TEST_CASE("Gini is 0 for perfectly equal wealth") {
    tge::AvlTree<int64_t, tge::HumanId> wealth;
    for (int i = 0; i < 100; ++i) {
        int64_t key = (int64_t(50) << 32) | (uint32_t)i;
        wealth.insert(key, i);
    }
    double g = tge::giniFromWealthIndex(wealth);
    CHECK(std::fabs(g) < 1e-9);
}

TEST_CASE("Gini approaches 1 when one person owns everything") {
    tge::AvlTree<int64_t, tge::HumanId> wealth;
    for (int i = 0; i < 100; ++i) {
        int w = (i == 99) ? 1000000 : 0;
        int64_t key = (int64_t(w) << 32) | (uint32_t)i;
        wealth.insert(key, i);
    }
    double g = tge::giniFromWealthIndex(wealth);
    CHECK(g > 0.98);
    CHECK(g <= 1.0);
}

TEST_CASE("Gini of sorted helper matches known values") {
    tge::DynArray<int> x;
    x.push_back(1); x.push_back(2); x.push_back(3); x.push_back(4);
    double g = tge::giniOfSorted(x);
    CHECK(g > 0.0);
    CHECK(g < 1.0);
}
