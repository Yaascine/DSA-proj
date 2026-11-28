// Simulation.h — the deterministic engine (P1).
// Purpose: own the systems (fixed order), command queue (our Queue), scheduler (our
//          MinHeap), four RNG streams, world, humans, history; publish a Snapshot.
// Tick order (enforced): 1) apply commands, 2) pop due scheduled events,
//   3) run registered systems in order (environment -> humans -> interactions),
//   4) publish Snapshot, 5) advance tick.
// API: step(), run(n), pushCommand(cmd), stateHash(), snapshot().
// Complexity: step O(systems + due events + commands).
// Who uses it: P1 headless runner; P4 gui drives it via Commands.
#pragma once
#include "core/common/Types.h"
#include "core/common/Rng.h"
#include "core/common/Fnv.h"
#include "core/dsa/DynArray.h"
#include "core/dsa/Queue.h"
#include "core/dsa/DsaStats.h"
#include "core/world/World.h"
#include "core/humans/Human.h"
#include "core/history/History.h"
#include "core/sim/Command.h"
#include "core/sim/Scheduler.h"
#include "core/sim/Snapshot.h"
#include "core/sim/ISystem.h"
#include "core/sim/SimContext.h"

namespace tge {

class Simulation {
    Tick tick_ = 0;
    uint64_t seed_ = 1;
    Rng rngWorld_, rngPop_, rngEvent_, rngNarr_;
    World world_;
    DynArray<Human> humans_;
    History history_;
    Scheduler scheduler_;
    Queue<Command> commands_;
    DynArray<ISystem*> systems_; // fixed registration order
    Metrics metrics_;
    DsaStats dsa_;
    Snapshot snap_;
    DynArray<HeapItem> due_;
    HumanId nextHumanId_ = 0;

    void applyCommands();
    void applyOne(const Command& c);
    void publishSnapshot();
    HumanId spawnHuman(TileCoord pos, bool playerCaused);

public:
    Simulation(uint64_t seed, int initialPop);

    void addSystem(ISystem* s) { systems_.push_back(s); }
    void pushCommand(const Command& c) { commands_.push(c); }
    void step();
    void run(int n) { for (int i = 0; i < n; ++i) step(); }

    uint64_t stateHash() const;
    const Snapshot& snapshot() const { return snap_; }
    Tick tick() const { return tick_; }
    void setTick(Tick t) { tick_ = t; } // persistence restore only
    uint64_t seed() const { return seed_; }

    // Access for persistence, tests and the headless demo.
    World& world() { return world_; }
    DynArray<Human>& humans() { return humans_; }
    History& history() { return history_; }
    Scheduler& scheduler() { return scheduler_; }
    Metrics& metrics() { return metrics_; }
    Rng& rngWorld() { return rngWorld_; }
    Rng& rngPop() { return rngPop_; }
    Rng& rngEvent() { return rngEvent_; }
    Rng& rngNarr() { return rngNarr_; }
    const World& world() const { return world_; }
    const DynArray<Human>& humans() const { return humans_; }
    const History& history() const { return history_; }
    const Scheduler& scheduler() const { return scheduler_; }
    const Metrics& metrics() const { return metrics_; }
    const Rng& rngWorld() const { return rngWorld_; }
    const Rng& rngPop() const { return rngPop_; }
    const Rng& rngEvent() const { return rngEvent_; }
    const Rng& rngNarr() const { return rngNarr_; }
    HumanId nextHumanId() const { return nextHumanId_; }
    void setNextHumanId(HumanId id) { nextHumanId_ = id; }

    // Canonical name generation (deterministic, population stream).
    static const char* randomName(Rng& pop);
};

} // namespace tge
