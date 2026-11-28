// SimulationTest.cpp — determinism: same seed + commands => same stateHash.
#include "doctest.h"
#include "core/sim/Simulation.h"
#include "tools/DemoSystem.h"

TEST_CASE("Two runs with the same seed give the same stateHash") {
    tge::Simulation a(42, 80), b(42, 80);
    tge::DemoSystem da, db;
    a.addSystem(&da); b.addSystem(&db);
    a.run(100); b.run(100);
    CHECK(a.stateHash() == b.stateHash());
    CHECK(a.snapshot().population() == b.snapshot().population());
}

TEST_CASE("Different seeds give different stateHash") {
    tge::Simulation a(42, 80), b(43, 80);
    tge::DemoSystem da, db;
    a.addSystem(&da); b.addSystem(&db);
    a.run(100); b.run(100);
    CHECK(a.stateHash() != b.stateHash());
}

TEST_CASE("Same seed + same command list give the same stateHash") {
    auto runWithCommands = [](uint64_t seed) {
        tge::Simulation sim(seed, 80);
        tge::DemoSystem demo;
        sim.addSystem(&demo);
        sim.run(50);
        tge::Command fire; fire.type = tge::CmdType::StartFire; fire.a = 32; fire.b = 32; fire.c = 5;
        sim.pushCommand(fire);
        tge::Command spawn; spawn.type = tge::CmdType::SpawnHuman; spawn.a = 30; spawn.b = 30;
        sim.pushCommand(spawn);
        sim.run(50);
        return sim.stateHash();
    };
    CHECK(runWithCommands(42) == runWithCommands(42));
}

TEST_CASE("A God-Mode command changes the state (GUI pushes, never mutates)") {
    tge::Simulation a(42, 80), b(42, 80);
    tge::DemoSystem da, db;
    a.addSystem(&da); b.addSystem(&db);
    a.run(50); b.run(50);
    tge::Command fire; fire.type = tge::CmdType::StartFire; fire.a = 32; fire.b = 32; fire.c = 9;
    b.pushCommand(fire);
    b.run(1); a.run(1);
    CHECK(a.stateHash() != b.stateHash());
    CHECK(b.history().size() > a.history().size()); // fire event recorded
}

TEST_CASE("Scheduler fires due events in (tick,seq) order") {
    tge::Simulation sim(1, 10);
    sim.scheduler().schedule(5, 100);
    sim.scheduler().schedule(5, 101);
    sim.scheduler().schedule(3, 102);
    tge::DynArray<tge::HeapItem> due;
    sim.scheduler().popDue(5, due);
    REQUIRE(due.size() == 3);
    CHECK(due[0].payload == 102); // tick 3 first
    CHECK(due[1].payload == 100); // tick 5, seq order
    CHECK(due[2].payload == 101);
}
