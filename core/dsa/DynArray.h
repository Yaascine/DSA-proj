// DynArray.h — hand-written dynamic array (the graded vector replacement).
// Purpose: contiguous, growable storage for entities, heap backing, node pools.
// Design: capacity DOUBLES on growth (amortised O(1) push_back); bounds-checked
//         operator[] in Debug builds. No STL containers used internally.
// Complexity: push_back amortised O(1); pop_back O(1); operator[] O(1);
//             insert/erase at index O(n); reserve O(n) when it grows.
// Who uses it: every other DSA structure + sim entity storage.
// Memory: raw new[]/delete[] live ONLY here (allowed: this is our own container).
#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <utility>
#include "DsaStats.h"

namespace tge {

template <typename T>
class DynArray {
    T* data_ = nullptr;
    size_t size_ = 0;
    size_t cap_ = 0;
    DsaStats* st_ = nullptr;

    void growTo(size_t newCap) {
        T* nd = new T[newCap];
        for (size_t i = 0; i < size_; ++i) nd[i] = std::move(data_[i]);
        delete[] data_;
        data_ = nd;
        cap_ = newCap;
        if (st_) ++st_->resizes;
    }

public:
    DynArray() = default;
    explicit DynArray(DsaStats* st) : st_(st) {}
    ~DynArray() { delete[] data_; }

    // Non-copyable to keep ownership obvious; movable for container-of-container use.
    DynArray(const DynArray&) = delete;
    DynArray& operator=(const DynArray&) = delete;
    DynArray(DynArray&& o) noexcept
        : data_(o.data_), size_(o.size_), cap_(o.cap_), st_(o.st_) {
        o.data_ = nullptr; o.size_ = 0; o.cap_ = 0;
    }
    DynArray& operator=(DynArray&& o) noexcept {
        if (this != &o) {
            delete[] data_;
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
        data_[size_++] = v;
        if (st_) ++st_->pushes;
    }
    void push_back(T&& v) {
        if (size_ == cap_) growTo(cap_ ? cap_ * 2 : 4);
        data_[size_++] = std::move(v);
        if (st_) ++st_->pushes;
    }
    template <typename... Args>
    T& emplace_back(Args&&... args) {
        if (size_ == cap_) growTo(cap_ ? cap_ * 2 : 4);
        data_[size_] = T(std::forward<Args>(args)...);
        return data_[size_++];
    }

    void pop_back() { assert(size_ > 0); --size_; if (st_) ++st_->pops; }

    void clear() { size_ = 0; }

    void resize(size_t n) {
        if (n > cap_) reserve(n);
        for (size_t i = size_; i < n; ++i) data_[i] = T{};
        size_ = n;
    }

    // Ordered insert/erase (shift). Used by small lists where order matters.
    void insertAt(size_t i, const T& v) {
        assert(i <= size_);
        if (size_ == cap_) growTo(cap_ ? cap_ * 2 : 4);
        for (size_t j = size_; j > i; --j) data_[j] = std::move(data_[j - 1]);
        data_[i] = v;
        ++size_;
        if (st_) ++st_->inserts;
    }
    void eraseAt(size_t i) {
        assert(i < size_);
        for (size_t j = i; j + 1 < size_; ++j) data_[j] = std::move(data_[j + 1]);
        --size_;
        if (st_) ++st_->erases;
    }
    // O(1) unordered removal: swap with last, then pop. Deterministic given ops.
    void swapRemoveAt(size_t i) {
        assert(i < size_);
        if (i != size_ - 1) { data_[i] = std::move(data_[size_ - 1]); if (st_) ++st_->swaps; }
        --size_;
        if (st_) ++st_->erases;
    }
};

} // namespace tge
