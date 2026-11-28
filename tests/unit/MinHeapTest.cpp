// MinHeapTest.cpp — 100k random ops vs std::priority_queue (min-heap, (key,seq) order).
#include "doctest.h"
#include <queue>
#include <vector>
#include "core/dsa/MinHeap.h"
#include "core/common/Rng.h"

namespace {
struct Cmp {
    bool operator()(const tge::HeapItem& a, const tge::HeapItem& b) const {
        return a.key > b.key || (a.key == b.key && a.seq > b.seq);
    }
};
}

TEST_CASE("MinHeap matches std::priority_queue over 100000 random ops") {
    tge::Rng r(7);
    tge::MinHeap h;
    std::priority_queue<tge::HeapItem, std::vector<tge::HeapItem>, Cmp> pq;
    int32_t seq = 0;
    for (int i = 0; i < 100000; ++i) {
        if (r.range(0, 9) < 6 || pq.empty()) {
            tge::HeapItem it{r.range(0, 1000), seq++, r.range(0, 1000000)};
            h.push(it); pq.push(it);
        } else {
            tge::HeapItem got = h.pop();
            tge::HeapItem want = pq.top(); pq.pop();
            CHECK(got.key == want.key);
            CHECK(got.seq == want.seq);
            CHECK(got.payload == want.payload);
        }
        REQUIRE(h.size() == pq.size());
    }
    while (!pq.empty()) {
        tge::HeapItem got = h.pop();
        tge::HeapItem want = pq.top(); pq.pop();
        CHECK(got.key == want.key);
        CHECK(got.seq == want.seq);
    }
}

TEST_CASE("MinHeap breaks ties by seq (FIFO for equal keys)") {
    tge::MinHeap h;
    h.push({5, 2, 20});
    h.push({5, 0, 10});
    h.push({5, 1, 15});
    h.push({1, 9, 99});
    CHECK(h.pop().payload == 99);
    CHECK(h.pop().payload == 10);
    CHECK(h.pop().payload == 15);
    CHECK(h.pop().payload == 20);
}
