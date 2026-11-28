// Scheduler.h — timed-event scheduler on our MinHeap.
// Purpose: pop events that are due on the current tick, in (dueTick, seq) order.
// Design: wraps MinHeap; assigns a monotonically increasing seq so equal-tick events
//         fire in insertion order (determinism). seq is part of saved state.
// Complexity: schedule O(log n); popDue O(k log n).
// Who uses it: P1 Simulation; P2/P3 schedule delayed effects.
#pragma once
#include "core/common/Types.h"
#include "core/common/Fnv.h"
#include "core/dsa/MinHeap.h"
#include "core/dsa/DynArray.h"
#include "core/dsa/DsaStats.h"

namespace tge {

class Scheduler {
    MinHeap heap_;
    int32_t nextSeq_ = 0;

public:
    Scheduler() = default;
    explicit Scheduler(DsaStats* st) { heap_.setStats(st); }
    void setStats(DsaStats* st) { heap_.setStats(st); }

    void schedule(Tick due, int32_t payload) { heap_.push(HeapItem{due, nextSeq_++, payload}); }
    size_t size() const { return heap_.size(); }
    bool empty() const { return heap_.empty(); }

    // Pop every item with key <= tick into `out` (ascending (key,seq) order).
    void popDue(Tick tick, DynArray<HeapItem>& out) {
        while (!heap_.empty() && heap_.peek().key <= tick) out.push_back(heap_.pop());
    }

    const MinHeap& heap() const { return heap_; }
    int32_t nextSeq() const { return nextSeq_; }

    void restore(const DynArray<HeapItem>& items, int32_t nextSeq) {
        heap_.restore(items);
        nextSeq_ = nextSeq;
    }
    void hashInto(Fnv& f) const {
        f.add32((uint32_t)nextSeq_);
        const auto& items = heap_.items();
        f.add64((uint64_t)items.size());
        for (size_t i = 0; i < items.size(); ++i) {
            f.add64((uint64_t)items[i].key);
            f.add32((uint32_t)items[i].seq);
            f.add32((uint32_t)items[i].payload);
        }
    }
};

} // namespace tge
