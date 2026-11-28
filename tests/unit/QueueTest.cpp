// QueueTest.cpp — 100k random ops vs std::queue (FIFO, circular buffer growth).
#include "doctest.h"
#include <queue>
#include "core/dsa/Queue.h"
#include "core/common/Rng.h"

TEST_CASE("Queue matches std::queue over 100000 random ops") {
    tge::Rng r(77);
    tge::Queue<int> q;
    std::queue<int> ref;
    for (int i = 0; i < 100000; ++i) {
        if (r.range(0, 9) < 6 || ref.empty()) {
            int val = r.range(-1000000, 1000000);
            q.push(val); ref.push(val);
        } else {
            CHECK(q.front() == ref.front());
            int got = q.pop();
            int want = ref.front(); ref.pop();
            CHECK(got == want);
        }
        REQUIRE(q.size() == ref.size());
    }
    while (!ref.empty()) {
        CHECK(q.pop() == ref.front());
        ref.pop();
    }
    CHECK(q.empty());
}

TEST_CASE("Queue wraps around the circular buffer correctly") {
    tge::Queue<int> q;
    for (int i = 0; i < 5; ++i) q.push(i);
    for (int i = 0; i < 3; ++i) CHECK(q.pop() == i);
    for (int i = 5; i < 20; ++i) q.push(i); // forces growth + wrap
    int expect = 3;
    while (!q.empty()) CHECK(q.pop() == expect++);
    CHECK(expect == 20);
}
