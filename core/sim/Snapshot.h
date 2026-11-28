// Snapshot.h — read-only published state for the GUI (P1 builds; P4 reads).
// Purpose: the GUI's ONLY view of the simulation. It never mutates state; it pushes
//          Commands. P1 finalised the sketch to use our DynArray for humans/events.
// Complexity: O(1) to read; O(window) to publish recent events.
// Who uses it: P4 gui.
#pragma once
#include "core/common/Types.h"
#include "core/common/Constants.h"
#include "core/dsa/DynArray.h"
#include "core/dsa/DsaStats.h"
#include "core/world/World.h"
#include "core/humans/Human.h"
#include "core/history/History.h"

namespace tge {

// Aggregate counters the diagnostics panel shows (real numbers, not estimates).
struct Metrics {
    int population = 0;
    int births = 0, deaths = 0, thefts = 0, fires = 0, floods = 0;
    int rumors = 0, scandals = 0, marriages = 0, rescues = 0;
};

struct Snapshot {
    Tick tick = 0;
    const World* world = nullptr;
    const DynArray<Human>* humans = nullptr;
    const History* history = nullptr;
    DynArray<EventId> recentEvents; // last SNAPSHOT_EVENT_WINDOW event ids
    Metrics metrics;
    DsaStats dsa;                   // aggregate operation counters
    uint64_t stateHash = 0;

    int population() const {
        if (!humans) return 0;
        int n = 0;
        for (size_t i = 0; i < humans->size(); ++i) if ((*humans)[i].alive) ++n;
        return n;
    }
};

} // namespace tge
