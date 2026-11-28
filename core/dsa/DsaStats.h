// DsaStats.h — operation counters shared by every hand-written structure.
// Purpose: feed the P4 diagnostics panel with REAL numbers (inserts, probes,
//          collisions, rotations, resizes, comparisons, swaps, ...).
// Complexity: O(1) per increment.
// Who uses it: all core/dsa structures; P4 reads the aggregate via Snapshot.
// Each structure holds a DsaStats* (may be null in tests that don't care).
#pragma once
#include <cstdint>

namespace tge {

struct DsaStats {
    uint64_t inserts     = 0;
    uint64_t erases      = 0;
    uint64_t lookups     = 0;
    uint64_t probes      = 0;   // hash bucket / slot inspections
    uint64_t collisions  = 0;   // hash chain steps beyond the first
    uint64_t rotations   = 0;   // AVL rebalance rotations
    uint64_t resizes     = 0;   // capacity growth events
    uint64_t comparisons = 0;   // key comparisons
    uint64_t swaps       = 0;   // element swaps / moves
    uint64_t pushes      = 0;   // queue/heap/list appends
    uint64_t pops        = 0;   // queue/heap/list removals

    void reset() { *this = DsaStats{}; }

    // Accumulate another structure's counters (for the diagnostics aggregate).
    void add(const DsaStats& o) {
        inserts += o.inserts; erases += o.erases; lookups += o.lookups;
        probes += o.probes; collisions += o.collisions; rotations += o.rotations;
        resizes += o.resizes; comparisons += o.comparisons; swaps += o.swaps;
        pushes += o.pushes; pops += o.pops;
    }
};

// Small helper so structures can count without null-checking everywhere.
inline void bump(uint64_t& counter, DsaStats* st, uint64_t DsaStats::*field) {
    ++counter;
    if (st) ++(st->*field);
}

} // namespace tge
