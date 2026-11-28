// PersistenceCommandsTest.cpp — save/load mid-run WITH God-Mode commands preserves hash.
#include "doctest.h"
#include "core/sim/Simulation.h"
#include "core/persistence/SaveLoad.h"
#include "tools/DemoSystem.h"

namespace {
void commandsAt(tge::Simulation& s, int tick) {
    if (tick == 30) { tge::Command c; c.type = tge::CmdType::StartFire; c.a = 32; c.b = 32; c.c = 8; s.pushCommand(c); }
    if (tick == 60) { tge::Command c; c.type = tge::CmdType::SpawnHuman; c.a = 30; c.b = 30; s.pushCommand(c); }
    if (tick == 90) { tge::Command c; c.type = tge::CmdType::AddResource; c.a = 32; c.b = 32; c.c = (int)tge::Resource::Food; c.d = 200; s.pushCommand(c); }
    if (tick == 120) { tge::Command c; c.type = tge::CmdType::StartFlood; c.a = 20; c.b = 20; c.c = 70; s.pushCommand(c); }
}
}

TEST_CASE("Save/load with commands: 150 continuous == 75 + save + load + 75") {
    tge::Simulation continuous(42, 40);
    tge::DemoSystem dc;
    continuous.addSystem(&dc);
    for (int t = 0; t < 150; ++t) { commandsAt(continuous, t); continuous.step(); }
    uint64_t hashContinuous = continuous.stateHash();

    tge::Simulation part(42, 40);
    tge::DemoSystem dp;
    part.addSystem(&dp);
    for (int t = 0; t < 75; ++t) { commandsAt(part, t); part.step(); }
    std::string saved = tge::SaveLoad::toJson(part);

    tge::Simulation restored(42, 0);
    tge::DemoSystem dr;
    restored.addSystem(&dr);
    REQUIRE(tge::SaveLoad::fromJson(restored, saved));
    for (int t = 75; t < 150; ++t) { commandsAt(restored, t); restored.step(); }

    CHECK(restored.stateHash() == hashContinuous);
}
