// HashTableTest.cpp — 100k random ops vs std::unordered_map + iteration determinism.
#include "doctest.h"
#include <unordered_map>
#include <vector>
#include "core/dsa/HashTable.h"
#include "core/common/Rng.h"

TEST_CASE("HashTable matches std::unordered_map over 100000 random ops") {
    tge::Rng r(99);
    tge::HashTable<int> h;
    std::unordered_map<int32_t, int> m;
    for (int i = 0; i < 100000; ++i) {
        int32_t key = r.range(0, 5000);
        int op = r.range(0, 9);
        if (op < 5) {
            int val = r.range(-1000000, 1000000);
            h.put(key, val); m[key] = val;
        } else if (op < 8) {
            int got = -1; bool found = h.get(key, got);
            auto it = m.find(key);
            CHECK(found == (it != m.end()));
            if (found) CHECK(got == it->second);
        } else {
            bool er = h.erase(key);
            bool erm = m.erase(key) > 0;
            CHECK(er == erm);
        }
        REQUIRE(h.size() == m.size());
    }
    // Final membership cross-check.
    for (auto& kv : m) { int got = 0; CHECK(h.get(kv.first, got)); CHECK(got == kv.second); }
}

TEST_CASE("HashTable iteration order is identical across two runs") {
    auto run = []() {
        tge::Rng r(555);
        tge::HashTable<int> h;
        for (int i = 0; i < 20000; ++i) {
            int32_t key = r.range(0, 3000);
            if (r.range(0, 9) < 7) h.put(key, r.range(0, 100000));
            else h.erase(key);
        }
        std::vector<int32_t> order;
        for (auto& e : h) order.push_back(e.key);
        return order;
    };
    std::vector<int32_t> a = run(), b = run();
    REQUIRE(a.size() == b.size());
    for (size_t i = 0; i < a.size(); ++i) CHECK(a[i] == b[i]);
}

TEST_CASE("HashTable grows past load factor 0.75") {
    tge::HashTable<int> h;
    for (int i = 0; i < 1000; ++i) h.put(i, i * i);
    CHECK(h.size() == 1000);
    CHECK(h.bucketCount() >= 1024);
    for (int i = 0; i < 1000; ++i) { int g = 0; CHECK(h.get(i, g)); CHECK(g == i * i); }
}
