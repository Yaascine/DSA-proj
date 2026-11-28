// DynArrayTest.cpp — 100k random ops vs std::vector.
#include "doctest.h"
#include <vector>
#include "core/dsa/DynArray.h"
#include "core/common/Rng.h"

TEST_CASE("DynArray matches std::vector over 100000 random ops") {
    tge::Rng r(123);
    tge::DynArray<int> a;
    std::vector<int> v;
    for (int i = 0; i < 100000; ++i) {
        int op = r.range(0, 9);
        if (op < 5 || v.empty()) {
            int val = r.range(-1000000, 1000000);
            a.push_back(val); v.push_back(val);
        } else if (op < 7) {
            a.pop_back(); v.pop_back();
        } else if (op < 9) {
            size_t idx = (size_t)r.range(0, (int)v.size() - 1);
            int val = r.range(-1000000, 1000000);
            a.insertAt(idx, val); v.insert(v.begin() + idx, val);
        } else {
            size_t idx = (size_t)r.range(0, (int)v.size() - 1);
            a.eraseAt(idx); v.erase(v.begin() + idx);
        }
        REQUIRE(a.size() == v.size());
        if (i % 2000 == 0)
            for (size_t j = 0; j < v.size(); ++j) CHECK(a[j] == v[j]);
    }
    for (size_t j = 0; j < v.size(); ++j) CHECK(a[j] == v[j]);
}

TEST_CASE("DynArray doubling capacity and swapRemove") {
    tge::DynArray<int> a;
    for (int i = 0; i < 1000; ++i) a.push_back(i);
    CHECK(a.size() == 1000);
    CHECK(a.capacity() >= 1000);
    a.swapRemoveAt(0);
    CHECK(a.size() == 999);
    CHECK(a[0] == 999); // last moved into slot 0
}
