// MinHeap.h — hand-written array-based binary MIN-heap (non-template, readable).
// Purpose: the event scheduler and P2's Dijkstra priority queue.
// Design: key is (priority, seq). Ties break by seq so equal-priority events pop
//         in insertion order — a missing tiebreaker is the classic determinism bug.
// Complexity: push O(log n); pop O(log n); peek/size/empty O(1).
// Who uses it: P1 scheduler; P2 Dijkstra.
// Backed by our DynArray; counts comparisons and swaps in DsaStats.
#pragma once
#include <cstdint>
#include <cassert>
#include <utility>
#include "DynArray.h"
#include "DsaStats.h"

namespace tge {

struct HeapItem {
    int64_t key = 0;      // priority (e.g. due tick)
    int32_t seq = 0;      // tiebreaker: insertion sequence
    int32_t payload = 0;  // user data (event id, node id, ...)
};

class MinHeap {
    DynArray<HeapItem> a_;
    DsaStats* st_ = nullptr;

    static bool less(const HeapItem& x, const HeapItem& y) {
        return x.key < y.key || (x.key == y.key && x.seq < y.seq);
    }
    void siftUp(size_t i) {
        while (i > 0) {
            size_t p = (i - 1) / 2;
            if (st_) ++st_->comparisons;
            if (!less(a_[i], a_[p])) break;
            std::swap(a_[i], a_[p]);
            if (st_) ++st_->swaps;
            i = p;
        }
    }
    void siftDown(size_t i) {
        size_t n = a_.size();
        for (;;) {
            size_t l = 2 * i + 1, r = l + 1, m = i;
            if (l < n) { if (st_) ++st_->comparisons; if (less(a_[l], a_[m])) m = l; }
            if (r < n) { if (st_) ++st_->comparisons; if (less(a_[r], a_[m])) m = r; }
            if (m == i) break;
            std::swap(a_[i], a_[m]);
            if (st_) ++st_->swaps;
            i = m;
        }
    }

public:
    MinHeap() = default;
    explicit MinHeap(DsaStats* st) : st_(st) { a_.setStats(st); }
    void setStats(DsaStats* st) { st_ = st; a_.setStats(st); }

    size_t size() const { return a_.size(); }
    bool empty() const { return a_.empty(); }

    void push(HeapItem it) {
        a_.push_back(it);
        siftUp(a_.size() - 1);
        if (st_) ++st_->pushes;
    }
    HeapItem pop() {
        assert(!a_.empty());
        HeapItem top = a_[0];
        HeapItem last = a_.back();
        a_.pop_back();
        if (!a_.empty()) { a_[0] = last; siftDown(0); }
        if (st_) ++st_->pops;
        return top;
    }
    const HeapItem& peek() const { assert(!a_.empty()); return a_[0]; }

    // Read-only access for save/load (canonical order = array order).
    const DynArray<HeapItem>& items() const { return a_; }
    void clear() { a_.clear(); }
    // Restore from a saved array (already heap-ordered).
    void restore(const DynArray<HeapItem>& src) {
        a_.clear();
        for (size_t i = 0; i < src.size(); ++i) a_.push_back(src[i]);
    }
};

} // namespace tge
