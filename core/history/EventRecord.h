// EventRecord.h — one immutable historical event.
// Purpose: the atom of the history log and the cause graph.
// Hard rule: causeIds are ONLY ever passed explicitly by a rule (e.g. a theft cites
//            the hunger event that triggered it). Cause is never inferred from timing.
// Complexity: O(actors + causes) to store.
// Who uses it: P1 history/narrator/persistence; P4 gui.
#pragma once
#include "core/common/Types.h"
#include "core/dsa/DynArray.h"
#include "core/history/EventType.h"

namespace tge {

struct EventRecord {
    EventId id = -1;
    Tick tick = 0;
    EventType type = EventType::Birth;
    DynArray<HumanId> actors;
    TileCoord where{0, 0};
    int param = 0;
    DynArray<EventId> causes;
    uint32_t flags = EF_NONE;
};

} // namespace tge
