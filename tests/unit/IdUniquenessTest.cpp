// IdUniquenessTest.cpp — regression for the duplicate-ID bug (births + spawns).
#include "doctest.h"
#include "core/sim/Simulation.h"
#include "tools/DemoSystem.h"

TEST_CASE("birthHuman allocates from the single ID authority") {
    tge::Simulation sim(1, 5); // nextHumanId == 5 after initial population
    tge::HumanId a = sim.birthHuman({1, 1});
    tge::HumanId b = sim.birthHuman({2, 2});
    CHECK(a == 5);
    CHECK(b == 6);
    CHECK(sim.nextHumanId() == 7);
    CHECK(sim.humans().size() == 7);
}

TEST_CASE("Human IDs stay unique across births AND God-Mode spawns") {
    tge::Simulation sim(42, 20);
    tge::DemoSystem demo;
    sim.addSystem(&demo);
    sim.run(80); // natural births occur here

    for (int i = 0; i < 10; ++i) {
        tge::Command c; c.type = tge::CmdType::SpawnHuman; c.a = 32; c.b = 32;
        sim.pushCommand(c);
    }
    sim.run(20); // more births + the spawns applied

    const auto& hs = sim.humans();
    tge::DynArray<bool> seen;
    seen.resize(hs.size());
    for (size_t i = 0; i < hs.size(); ++i) {
        tge::HumanId id = hs[i].id;
        REQUIRE(id >= 0);
        REQUIRE((size_t)id < hs.size());
        CHECK((size_t)id == i);          // id == index invariant
        CHECK_FALSE(seen[id]);           // no duplicate IDs
        seen[id] = true;
    }
    CHECK(sim.nextHumanId() == (tge::HumanId)hs.size());
}
