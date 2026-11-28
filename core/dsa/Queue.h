// Queue.h — hand-written FIFO queue on a circular buffer.
// Purpose: the command queue and BFS frontiers (rumors, evacuation).
// Design: ring buffer with head/tail indices; capacity DOUBLES when full (fixed
//         growth policy). Elements are moved into the new buffer in FIFO order.
// Complexity: push O(1) amortised; pop O(1); front O(1); size O(1).
// Who uses it: P1 command queue; P2/P3 BFS.
// No STL containers internally.
#pragma once
#include <cstddef>
#include <cassert>
#include <utility>
#include "DynArray.h"
#include "DsaStats.h"

namespace tge {

template <typename T>
class Queue {
    T* buf_ = nullptr;
    size_t cap_ = 0;
    size_t head_ = 0;   // index of front element
    size_t count_ = 0;
    DsaStats* st_ = nullptr;

    void grow() {
        size_t nc = cap_ ? cap_ * 2 : 4;
        T* nb = new T[nc];
        for (size_t i = 0; i < count_; ++i)
            nb[i] = std::move(buf_[(head_ + i) % cap_]);
        delete[] buf_;
        buf_ = nb; cap_ = nc; head_ = 0;
        if (st_) ++st_->resizes;
    }

public:
    Queue() = default;
    explicit Queue(DsaStats* st) : st_(st) {}
    ~Queue() { delete[] buf_; }
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;
    void setStats(DsaStats* st) { st_ = st; }

    size_t size() const { return count_; }
    bool empty() const { return count_ == 0; }

    void push(const T& v) {
        if (count_ == cap_) grow();
        buf_[(head_ + count_) % cap_] = v;
        ++count_;
        if (st_) ++st_->pushes;
    }
    void push(T&& v) {
        if (count_ == cap_) grow();
        buf_[(head_ + count_) % cap_] = std::move(v);
        ++count_;
        if (st_) ++st_->pushes;
    }
    T pop() {
        assert(count_ > 0);
        T v = std::move(buf_[head_]);
        head_ = (head_ + 1) % cap_;
        --count_;
        if (st_) ++st_->pops;
        return v;
    }
    const T& front() const { assert(count_ > 0); return buf_[head_]; }
    void clear() { head_ = 0; count_ = 0; }
};

} // namespace tge
