// AvlTreeTest.cpp — 100k random ops vs std::map + balance and in-order checks.
#include "doctest.h"
#include <map>
#include <cmath>
#include "core/dsa/AvlTree.h"
#include "core/common/Rng.h"

TEST_CASE("AvlTree matches std::map over 100000 random ops") {
    tge::Rng r(2024);
    tge::AvlTree<int, int> t;
    std::map<int, int> ref;
    for (int i = 0; i < 100000; ++i) {
        int key = r.range(0, 8000);
        int op = r.range(0, 9);
        if (op < 5) {
            int val = r.range(-1000000, 1000000);
            t.insert(key, val); ref[key] = val;
        } else if (op < 8) {
            int got = 0; bool found = t.find(key, got);
            auto it = ref.find(key);
            CHECK(found == (it != ref.end()));
            if (found) CHECK(got == it->second);
        } else {
            bool er = t.erase(key);
            bool erm = ref.erase(key) > 0;
            CHECK(er == erm);
        }
        REQUIRE(t.size() == ref.size());
    }
    // In-order walk must equal std::map iteration (sorted by key).
    tge::DynArray<tge::AvlTree<int, int>::KV> out;
    t.inOrder(out);
    REQUIRE(out.size() == ref.size());
    auto it = ref.begin();
    for (size_t i = 0; i < out.size(); ++i) {
        CHECK(out[i].key == it->first);
        CHECK(out[i].value == it->second);
        ++it;
    }
    // AVL height bound: <= ~1.44 log2(n+2). Use a generous safe ceiling.
    if (t.size() > 1) {
        double bound = 2.0 * std::log2((double)t.size() + 2.0) + 2.0;
        CHECK((double)t.treeHeight() <= bound);
    }
}

TEST_CASE("AvlTree reverse in-order is descending") {
    tge::AvlTree<int, int> t;
    for (int i = 0; i < 1000; ++i) t.insert(i, i * 10);
    tge::DynArray<tge::AvlTree<int, int>::KV> out;
    t.reverseInOrder(out);
    REQUIRE(out.size() == 1000);
    for (size_t i = 0; i + 1 < out.size(); ++i) CHECK(out[i].key > out[i + 1].key);
    CHECK(out[0].key == 999);
}

TEST_CASE("AvlTree stays balanced under ascending inserts (worst case for plain BST)") {
    tge::AvlTree<int, int> t;
    for (int i = 0; i < 10000; ++i) t.insert(i, i);
    CHECK(t.size() == 10000);
    CHECK(t.treeHeight() <= 20); // a plain BST would be 10000 tall
}
