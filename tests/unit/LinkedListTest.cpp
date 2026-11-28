// LinkedListTest.cpp — 100k random ops vs std::list (push front/back, remove by position).
#include "doctest.h"
#include <list>
#include "core/dsa/LinkedList.h"
#include "core/dsa/DynArray.h"
#include "core/common/Rng.h"

TEST_CASE("LinkedList matches std::list over 100000 random ops") {
    tge::Rng r(31);
    tge::LinkedList<int> l;
    std::list<int> ref;
    tge::DynArray<tge::LinkedList<int>::Handle> handles; // parallel to ref order

    for (int i = 0; i < 100000; ++i) {
        int op = r.range(0, 9);
        if (op < 4 || ref.empty()) {
            int val = r.range(-1000000, 1000000);
            if (op % 2 == 0) {
                auto h = l.pushBack(val); ref.push_back(val); handles.push_back(h);
            } else {
                auto h = l.pushFront(val); ref.push_front(val); handles.insertAt(0, h);
            }
        } else {
            size_t idx = (size_t)r.range(0, (int)ref.size() - 1);
            auto it = ref.begin(); std::advance(it, idx);
            ref.erase(it);
            l.remove(handles[idx]);
            handles.eraseAt(idx);
        }
        REQUIRE(l.size() == ref.size());
        REQUIRE(handles.size() == ref.size());
        if (i % 2000 == 0) {
            auto rit = ref.begin();
            for (int v : l) { CHECK(v == *rit); ++rit; }
        }
    }
    auto rit = ref.begin();
    for (int v : l) { CHECK(v == *rit); ++rit; }
}

TEST_CASE("LinkedList O(1) remove by handle keeps order") {
    tge::LinkedList<int> l;
    auto a = l.pushBack(1);
    auto b = l.pushBack(2);
    auto c = l.pushBack(3);
    l.remove(b);
    CHECK(l.size() == 2);
    CHECK(l.front() == 1);
    CHECK(l.back() == 3);
    l.remove(a);
    CHECK(l.front() == 3);
    l.remove(c);
    CHECK(l.empty());
}
