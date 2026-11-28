// NarratorTest.cpp — narration from recorded facts only; narration RNG independence.
#include "doctest.h"
#include "core/history/Narrator.h"
#include "core/history/History.h"
#include "core/humans/Human.h"
#include "core/dsa/DynArray.h"

namespace {
tge::DynArray<tge::Human> makeHumans() {
    tge::DynArray<tge::Human> hs;
    tge::Human a; a.id = 0; a.name = "Ada"; a.alive = true; a.ageDays = 40 * 365;
    tge::Human b; b.id = 1; b.name = "Bram"; b.alive = true; b.ageDays = 30 * 365;
    hs.push_back(a); hs.push_back(b);
    return hs;
}
}

TEST_CASE("Narrator narrates a theft using actor names from recorded data") {
    tge::History h;
    auto hunger = h.record(tge::EventType::Hunger, 5, {0}, {1, 1}, 90, {}, tge::EF_VERIFIED);
    auto theft = h.record(tge::EventType::Theft, 6, {0, 1}, {1, 1}, 3, {hunger}, tge::EF_VERIFIED);
    auto hs = makeHumans();
    tge::Narrator nar;
    tge::Rng narr(7);
    std::string text = nar.narrate(h.get(theft), h, hs, narr);
    CHECK(text.find("Ada") != std::string::npos);
    CHECK(text.find("Bram") != std::string::npos);
    CHECK(!text.empty());
}

TEST_CASE("Narrator does NOT consume the simulation RNG streams") {
    tge::History h;
    auto theft = h.record(tge::EventType::Theft, 6, {0, 1}, {1, 1}, 3, {}, tge::EF_VERIFIED);
    auto hs = makeHumans();
    tge::Narrator nar;
    tge::Rng world(1), pop(2), event(3), narr(4);
    tge::Rng worldCopy = world, popCopy = pop, eventCopy = event;
    for (int i = 0; i < 20; ++i) nar.narrate(h.get(theft), h, hs, narr);
    CHECK(world == worldCopy);
    CHECK(pop == popCopy);
    CHECK(event == eventCopy);
}

TEST_CASE("Obituary is built only from recorded events") {
    tge::History h;
    h.record(tge::EventType::Birth, 0, {0}, {1, 1}, 0, {}, tge::EF_VERIFIED);
    h.record(tge::EventType::Feud, 100, {0, 1}, {1, 1}, 0, {}, tge::EF_VERIFIED);
    h.record(tge::EventType::Death, 40 * 365, {0}, {1, 1}, 0, {}, tge::EF_VERIFIED);
    auto hs = makeHumans();
    hs[0].alive = false;
    tge::Narrator nar;
    tge::Rng narr(7);
    std::string ob = nar.obituary(0, h, hs, narr);
    CHECK(ob.find("Ada") != std::string::npos);
    CHECK(ob.find("Cause of death: starvation") != std::string::npos);
    CHECK(ob.find("1 grudges") != std::string::npos);
    CHECK(ob.find("(unverified)") != std::string::npos);
}
