// HistoryTest.cpp — explicit causation: Drought -> CropFailure -> Hunger -> Theft.
#include "doctest.h"
#include "core/history/History.h"

TEST_CASE("traceRoots follows explicit causes back to the root") {
    tge::History h;
    auto drought = h.record(tge::EventType::Drought, 10, {}, {5, 5}, 0, {}, tge::EF_VERIFIED);
    auto crop    = h.record(tge::EventType::CropFailure, 12, {}, {5, 5}, 0, {drought}, tge::EF_VERIFIED);
    auto hunger  = h.record(tge::EventType::Hunger, 14, {1}, {5, 5}, 90, {crop}, tge::EF_VERIFIED);
    auto theft   = h.record(tge::EventType::Theft, 15, {1, 2}, {5, 5}, 3, {hunger}, tge::EF_VERIFIED);

    auto roots = h.traceRoots(theft);
    REQUIRE(roots.size() == 1);
    CHECK(roots[0] == drought);

    auto path = h.chain(drought, theft);
    REQUIRE(path.size() == 4);
    CHECK(path[0] == drought);
    CHECK(path[1] == crop);
    CHECK(path[2] == hunger);
    CHECK(path[3] == theft);
}

TEST_CASE("no cause edge is inferred from timing alone") {
    tge::History h;
    auto a = h.record(tge::EventType::Fire, 5, {}, {1, 1}, 0, {}, tge::EF_VERIFIED);
    auto b = h.record(tge::EventType::Theft, 5, {1}, {1, 1}, 0, {}, tge::EF_VERIFIED); // same tick, NO cause
    CHECK(!h.causeGraph().hasEdge(a, b));
    auto roots = h.traceRoots(b);
    REQUIRE(roots.size() == 1);
    CHECK(roots[0] == b); // it is its own root
}

TEST_CASE("eventsFor indexes events per human") {
    tge::History h;
    auto e1 = h.record(tge::EventType::Birth, 0, {7}, {2, 2}, 0, {}, tge::EF_VERIFIED);
    auto e2 = h.record(tge::EventType::Theft, 9, {7, 3}, {2, 2}, 1, {}, tge::EF_VERIFIED);
    h.record(tge::EventType::Death, 20, {3}, {2, 2}, 0, {}, tge::EF_VERIFIED);
    const auto& list = h.eventsFor(7);
    REQUIRE(list.size() == 2);
    CHECK(list.front() == e1);
    CHECK(list.back() == e2);
    CHECK(h.eventsFor(999).empty());
}

TEST_CASE("multiple roots are returned in ascending order") {
    tge::History h;
    auto r1 = h.record(tge::EventType::Drought, 1, {}, {0, 0}, 0, {}, tge::EF_VERIFIED);
    auto r2 = h.record(tge::EventType::Fire, 2, {}, {0, 0}, 0, {}, tge::EF_VERIFIED);
    auto mid = h.record(tge::EventType::Famine, 3, {}, {0, 0}, 0, {r2, r1}, tge::EF_VERIFIED);
    auto roots = h.traceRoots(mid);
    REQUIRE(roots.size() == 2);
    CHECK(roots[0] == r1);
    CHECK(roots[1] == r2);
}
