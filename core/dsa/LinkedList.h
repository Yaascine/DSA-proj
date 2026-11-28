// LinkedList.h — hand-written DOUBLY linked list via an index-based node pool.
// Purpose: chronological history log and rumor path logs (append + iterate).
// Design: nodes live in a dense DynArray pool; prev/next are INDICES (no raw
//         pointers, no per-node new/delete). A free list reuses erased slots.
//         A "handle" is a node index. Append/remove-by-handle are O(1).
// Complexity: pushBack/pushFront O(1); remove(handle) O(1); iteration O(n).
// Who uses it: P1 history (per-human event lists), rumor paths.
// Iteration head->tail is deterministic given the same operation sequence.
#pragma once
#include <cstdint>
#include <cassert>
#include <utility>
#include "DynArray.h"
#include "DsaStats.h"

namespace tge {

template <typename T>
class LinkedList {
public:
    using Handle = int32_t;
    static constexpr Handle NONE = -1;
    struct Node {
        T value{};
        Handle prev = NONE;
        Handle next = NONE;
        bool alive = false;
    };

private:
    DynArray<Node> pool_;
    Handle head_ = NONE;
    Handle tail_ = NONE;
    Handle freeHead_ = NONE;
    size_t size_ = 0;
    DsaStats* st_ = nullptr;

    Handle allocNode() {
        if (freeHead_ != NONE) {
            Handle h = freeHead_;
            freeHead_ = pool_[h].next;
            return h;
        }
        pool_.push_back(Node{});
        return (Handle)pool_.size() - 1;
    }

public:
    LinkedList() = default;
    explicit LinkedList(DsaStats* st) : st_(st) { pool_.setStats(st); }
    void setStats(DsaStats* st) { st_ = st; pool_.setStats(st); }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    Handle head() const { return head_; }
    Handle tail() const { return tail_; }

    Handle pushBack(const T& v) {
        Handle h = allocNode();
        Node& n = pool_[h];
        n.value = v; n.alive = true; n.prev = tail_; n.next = NONE;
        if (tail_ != NONE) pool_[tail_].next = h; else head_ = h;
        tail_ = h;
        ++size_;
        if (st_) ++st_->pushes;
        return h;
    }
    Handle pushFront(const T& v) {
        Handle h = allocNode();
        Node& n = pool_[h];
        n.value = v; n.alive = true; n.prev = NONE; n.next = head_;
        if (head_ != NONE) pool_[head_].prev = h; else tail_ = h;
        head_ = h;
        ++size_;
        if (st_) ++st_->pushes;
        return h;
    }
    void remove(Handle h) {
        assert(h != NONE && pool_[h].alive);
        Node& n = pool_[h];
        if (n.prev != NONE) pool_[n.prev].next = n.next; else head_ = n.next;
        if (n.next != NONE) pool_[n.next].prev = n.prev; else tail_ = n.prev;
        n.alive = false;
        n.next = freeHead_;
        freeHead_ = h;
        --size_;
        if (st_) ++st_->erases;
    }
    T& value(Handle h) { assert(h != NONE && pool_[h].alive); return pool_[h].value; }
    const T& value(Handle h) const { assert(h != NONE && pool_[h].alive); return pool_[h].value; }
    Handle next(Handle h) const { return pool_[h].next; }
    Handle prev(Handle h) const { return pool_[h].prev; }
    const T& front() const { assert(size_ > 0); return pool_[head_].value; }
    const T& back() const { assert(size_ > 0); return pool_[tail_].value; }

    // Forward iterators over live nodes, head -> tail (mutable + const).
    class Iter {
        LinkedList* l_;
        Handle h_;
    public:
        Iter(LinkedList* l, Handle h) : l_(l), h_(h) {}
        T& operator*() const { return l_->pool_[h_].value; }
        Handle handle() const { return h_; }
        Iter& operator++() { h_ = l_->pool_[h_].next; return *this; }
        bool operator!=(const Iter& o) const { return h_ != o.h_; }
    };
    class ConstIter {
        const LinkedList* l_;
        Handle h_;
    public:
        ConstIter(const LinkedList* l, Handle h) : l_(l), h_(h) {}
        const T& operator*() const { return l_->pool_[h_].value; }
        Handle handle() const { return h_; }
        ConstIter& operator++() { h_ = l_->pool_[h_].next; return *this; }
        bool operator!=(const ConstIter& o) const { return h_ != o.h_; }
    };
    Iter begin() { return Iter(this, head_); }
    Iter end() { return Iter(this, NONE); }
    ConstIter begin() const { return ConstIter(this, head_); }
    ConstIter end() const { return ConstIter(this, NONE); }
};

} // namespace tge
