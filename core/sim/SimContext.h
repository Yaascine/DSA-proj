// SimContext.h — everything a system needs during one tick.
// Purpose: pass the four RNG streams, world, humans, history, scheduler, the events
//          that fired this tick, and metrics to each ISystem::onTick.
// Determinism: systems must use the provided streams (never rand()/chrono) and
//              iterate humans in ascending ID order.
// Complexity: O(1) plain references.
// Who uses it: P2/P3 systems.
#pragma once
#include "core/common/Types.h"
#include "core/common/Rng.h"
#include "core/dsa/DynArray.h"
#include "core/dsa/MinHeap.h"
#include "core/world/World.h"
#include "core/humans/Human.h"
#include "core/history/History.h"
#include "core/sim/Scheduler.h"
#include "core/sim/Snapshot.h"

namespace tge {

class Simulation; // forward declaration (systems may call the engine's ID authority)

struct SimContext {
    Tick tick = 0;
    Simulation* sim = nullptr; // engine pointer: use sim->birthHuman() for new humans
    Rng* world = nullptr;
    Rng* population = nullptr;
    Rng* event = nullptr;
    Rng* narration = nullptr;
    World* worldObj = nullptr;
    DynArray<Human>* humans = nullptr;
    History* history = nullptr;
    Scheduler* scheduler = nullptr;
    const DynArray<HeapItem>* dueEvents = nullptr; // scheduled items fired this tick
    Metrics* metrics = nullptr;
};

// NOTE: new humans must ALWAYS be created through Simulation::birthHuman()/spawn so
// IDs come from one authority (nextHumanId_). Never assign IDs from humans.size().

} // namespace tge
