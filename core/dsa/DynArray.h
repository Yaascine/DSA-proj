// DynArray.h — hand-written dynamic array (the graded vector replacement).
// Purpose: contiguous, growable storage for entities, heap backing, node pools.
// Design: capacity DOUBLES on growth (amortised O(1) push_back); bounds-checked
//         operator[] in Debug builds. No STL containers used internally.
//         Proper object lifetime: raw storage + placement-new on insert and explicit
//         destructor calls on removal, so non-trivial elements (e.g. std::string
//         names) release their resources immediately and ctor/dtor counts balance.
// Complexity: push_back amortised O(1); pop_back O(1); operator[] O(1);
//             insert/erase at index O(n); reserve O(n) when it grows.
// Who uses it: every other DSA structure + sim entity storage.
// Memory: raw allocation lives ONLY here (allowed: this is our own container).
#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <utility>
#include <new>
#include "DsaStats.h"

namespace tge {

template <typename T>
class DynArray {
    T* data_ = nullptr;   // raw storage; only [0, size_) hold live objects
    size_t size_ = 0;
    size_t cap_ = 0;
    DsaStats* st_ = nullptr;

    static T* allocRaw(size_t n) { return static_cast<T*>(::operator new(n * sizeof(T))); }
    static void freeRaw(T* p) { ::operator delete(p); }

    void growTo(size_t newCap) {
        T* nd = allocRaw(newCap);
        for (size_t i = 0; i < size_; ++i) {
            ::new (nd + i) T(std::move(data_[i])); // move-construct into new storage
            data_[i].~T();                         // destroy the old object
        }
        freeRaw(data_);
        data_ = nd;
        cap_ = newCap;
        if (st_) ++st_->resizes;
    }
    void destroyAll() { for (size_t i = 0; i < size_; ++i) data_[i].~T(); size_ = 0; }

public:
    DynArray() = default;
    explicit DynArray(DsaStats* st) : st_(st) {}
    ~DynArray() { destroyAll(); freeRaw(data_); }

    // Non-copyable to keep ownership obvious; movable for container-of-container use.
    DynArray(const DynArray&) = delete;
    DynArray& operator=(const DynArray&) = delete;
    DynArray(DynArray&& o) noexcept
        : data_(o.data_), size_(o.size_), cap_(o.cap_), st_(o.st_) {
        o.data_ = nullptr; o.size_ = 0; o.cap_ = 0;
    }
    DynArray& operator=(DynArray&& o) noexcept {
        if (this != &o) {
            destroyAll(); freeRaw(data_);
            data_ = o.data_; size_ = o.size_; cap_ = o.cap_; st_ = o.st_;
            o.data_ = nullptr; o.size_ = 0; o.cap_ = 0;
        }
        return *this;
    }

    void setStats(DsaStats* st) { st_ = st; }

    size_t size() const { return size_; }
    size_t capacity() const { return cap_; }
    bool empty() const { return size_ == 0; }
    T* data() { return data_; }
    const T* data() const { return data_; }

    T& operator[](size_t i) { assert(i < size_); return data_[i]; }
    const T& operator[](size_t i) const { assert(i < size_); return data_[i]; }
    T& back() { assert(size_ > 0); return data_[size_ - 1]; }
    const T& back() const { assert(size_ > 0); return data_[size_ - 1]; }

    // Range-for support (plain pointers, not STL iterators).
    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }

    void reserve(size_t n) { if (n > cap_) growTo(n); }

    void push_back(const T& v) {
        if (size_ == cap_) growTo(cap_ ? cap_ * 2 : 4);
        ::new (data_ + size_) T(v);
        ++size_;
        if (st_) ++st_->pushes;
    }
    void push_back(T&& v) {
        if (size_ == cap_) growTo(cap_ ? cap_ * 2 : 4);
        ::new (data_ + size_) T(std::move(v));
        ++size_;
        if (st_) ++st_->pushes;
    }
    template <typename... Args>
    T& emplace_back(Args&&... args) {
        if (size_ == cap_) growTo(cap_ ? cap_ * 2 : 4);
        ::new (data_ + size_) T(std::forward<Args>(args)...);
        return data_[size_++];
    }

    void pop_back() { assert(size_ > 0); --size_; data_[size_].~T(); if (st_) ++st_->pops; }

    void clear() { destroyAll(); }

    void resize(size_t n) {
        if (n > cap_) reserve(n);
        if (n > size_) for (size_t i = size_; i < n; ++i) ::new (data_ + i) T();
        else for (size_t i = n; i < size_; ++i) data_[i].~T();
        size_ = n;
    }

    // Ordered insert/erase (shift). Used by small lists where order matters.
    void insertAt(size_t i, const T& v) {
        assert(i <= size_);
        if (size_ == cap_) growTo(cap_ ? cap_ * 2 : 4);
        if (size_ == 0) { ::new (data_) T(v); size_ = 1; if (st_) ++st_->inserts; return; }
        // Construct the new tail from the old tail, shift the middle right, place v.
        ::new (data_ + size_) T(std::move(data_[size_ - 1]));
        for (size_t j = size_ - 1; j > i; --j) data_[j] = std::move(data_[j - 1]);
        data_[i] = v;
        ++size_;
        if (st_) ++st_->inserts;
    }
    void eraseAt(size_t i) {
        assert(i < size_);
        for (size_t j = i; j + 1 < size_; ++j) data_[j] = std::move(data_[j + 1]);
        --size_;
        data_[size_].~T();
        if (st_) ++st_->erases;
    }
    // O(1) unordered removal: swap with last, then pop. Deterministic given ops.
    void swapRemoveAt(size_t i) {
        assert(i < size_);
        if (i != size_ - 1) { data_[i] = std::move(data_[size_ - 1]); if (st_) ++st_->swaps; }
        --size_;
        data_[size_].~T();
        if (st_) ++st_->erases;
    }
};

} // namespace tge
