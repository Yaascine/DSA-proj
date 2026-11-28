// PersistenceTest.cpp — save/load preserves the canonical stateHash.
#include "doctest.h"
#include "core/sim/Simulation.h"
#include "core/persistence/SaveLoad.h"
#include "tools/DemoSystem.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdio>

TEST_CASE("200 continuous days == 100 days + save + load + 100 days") {
    tge::Simulation continuous(42, 80);
    tge::DemoSystem dc;
    continuous.addSystem(&dc);
    continuous.run(200);
    uint64_t hashContinuous = continuous.stateHash();

    tge::Simulation part(42, 80);
    tge::DemoSystem dp;
    part.addSystem(&dp);
    part.run(100);
    std::string saved = tge::SaveLoad::toJson(part);

    tge::Simulation restored(42, 0); // empty; state comes from the save
    tge::DemoSystem dr;
    restored.addSystem(&dr);
    REQUIRE(tge::SaveLoad::fromJson(restored, saved));
    restored.run(100);

    CHECK(restored.stateHash() == hashContinuous);
}

TEST_CASE("JSON round-trip preserves the hash immediately") {
    tge::Simulation sim(7, 60);
    tge::DemoSystem d;
    sim.addSystem(&d);
    sim.run(120);
    std::string json = tge::SaveLoad::toJson(sim);
    CHECK(json.find("\"schemaVersion\"") != std::string::npos);

    tge::Simulation back(7, 0);
    tge::DemoSystem d2;
    back.addSystem(&d2);
    REQUIRE(tge::SaveLoad::fromJson(back, json));
    CHECK(back.stateHash() == sim.stateHash());
    CHECK(back.tick() == sim.tick());
    CHECK(back.history().size() == sim.history().size());
}

TEST_CASE("File save/load round-trip preserves the hash") {
    tge::Simulation sim(99, 50);
    tge::DemoSystem d;
    sim.addSystem(&d);
    sim.run(80);
    // Portable temp path: independent of the test runner's working directory.
    std::string path = (std::filesystem::temp_directory_path() / "tge_test_save.json").string();
    REQUIRE(tge::SaveLoad::save(sim, path));

    tge::Simulation back(99, 0);
    tge::DemoSystem d2;
    back.addSystem(&d2);
    REQUIRE(tge::SaveLoad::loadInto(back, path));
    CHECK(back.stateHash() == sim.stateHash());
    std::remove(path.c_str());
}

TEST_CASE("Schema mismatch is rejected") {
    tge::Simulation sim(1, 5);
    std::string bad = "{\"schemaVersion\": 999}";
    CHECK_FALSE(tge::SaveLoad::fromJson(sim, bad));
}
