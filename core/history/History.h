// History.h — event log + cause graph + per-human index.
// Purpose: record events, trace explicit causation, build cheap obituaries.
// Design: events stored in a DynArray (id == index, ascending = chronological with
//         a (tick, id) tiebreaker). Cause graph is our Graph (edge cause -> effect,
//         in-edges kept for reverse BFS). eventsFor uses a per-human LinkedList of
//         EventIds so obituaries are O(events for that human).
// Complexity: record O(actors + causes); traceRoots/chain O(V + E) BFS;
//             eventsFor O(1) access + O(k) iterate.
// Who uses it: P1 sim/narrator/persistence; P4 gui.
#pragma once
#include <vector>
#include "core/common/Types.h"
#include "core/common/Fnv.h"
#include "core/dsa/DynArray.h"
#include "core/dsa/LinkedList.h"
#include "core/dsa/Graph.h"
#include "core/dsa/DsaStats.h"
#include "core/history/EventRecord.h"

namespace tge {

class History {
    DynArray<EventRecord> events_;
    Graph causeGraph_;
    DynArray<LinkedList<EventId>> perHuman_; // indexed by HumanId (sequential)
    DsaStats stats_;

    void ensureHuman(HumanId id);

public:
    History();

    // Contract signature (frozen). Returns the new EventId.
    EventId record(EventType type, Tick tick, std::vector<HumanId> actors, TileCoord where,
                   int param, std::vector<EventId> causes, uint32_t flags);

    size_t size() const { return events_.size(); }
    const EventRecord& get(EventId id) const { return events_[id]; }
    const DynArray<EventRecord>& events() const { return events_; }
    const Graph& causeGraph() const { return causeGraph_; }
    const DsaStats& stats() const { return stats_; }

    // Reverse BFS to events with no causes (the root causes of `id`).
    DynArray<EventId> traceRoots(EventId id) const;
    // Forward BFS path from `from` to `to` (inclusive), empty if unreachable.
    DynArray<EventId> chain(EventId from, EventId to) const;
    // Chronological events involving a human (for obituaries).
    const LinkedList<EventId>& eventsFor(HumanId id) const;

    void hashInto(Fnv& f) const;

    // Restore from saved events (rebuilds graph + per-human index).
    void restore(const DynArray<EventRecord>& src);
    void clear();
};

} // namespace tge
