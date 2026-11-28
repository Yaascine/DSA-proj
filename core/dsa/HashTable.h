// HashTable.h — hand-written hash table, int32 key -> V value.
// Purpose: ID -> human/settlement/event lookup that is DETERMINISTIC to iterate.
// Design: separate chaining via INDEX links (no pointers). Values live in a dense
//         entry array; buckets store head indices. Grows (doubles buckets) at load
//         factor 0.75. Erased slots go on a free list and are reused. Iteration
//         walks the dense array in slot order, skipping dead slots, so two runs
//         with the same operations always iterate identically (unlike unordered_map).
// Complexity: average O(1) put/get/erase; worst O(n) on a pathological chain;
//             rehash O(n + buckets).
// Who uses it: sim ID lookup, history eventsFor(HumanId).
// No STL containers internally.
#pragma once
#include <cstdint>
#include "DynArray.h"
#include "DsaStats.h"

namespace tge {

template <typename V>
class HashTable {
public:
    struct Entry {
        int32_t key = -1;
        V value{};
        int32_t next = -1;  // index of next entry in the same bucket chain
        bool alive = false;
    };

private:
    DynArray<int32_t> buckets_;   // head entry index per bucket, -1 = empty
    DynArray<Entry> entries_;     // dense storage (live + dead slots)
    size_t live_ = 0;
    int32_t freeHead_ = -1;       // singly-linked free slot list (via .next)
    DsaStats* st_ = nullptr;

    static uint32_t hashKey(int32_t k) {
        uint32_t x = (uint32_t)k;
        x ^= x >> 16; x *= 0x7feb352dU;
        x ^= x >> 15; x *= 0x846ca68bU;
        x ^= x >> 16;
        return x;
    }
    size_t bucketOf(int32_t k) const { return hashKey(k) & (buckets_.size() - 1); }

    void rehash(size_t newBucketCount) {
        buckets_.clear();
        buckets_.resize(newBucketCount);
        for (size_t i = 0; i < buckets_.size(); ++i) buckets_[i] = -1;
        // Rebuild chains by walking dense order (keeps iteration order stable).
        for (size_t i = 0; i < entries_.size(); ++i) {
            Entry& e = entries_[i];
            if (!e.alive) continue;
            size_t b = hashKey(e.key) & (newBucketCount - 1);
            e.next = buckets_[b];
            buckets_[b] = (int32_t)i;
        }
        if (st_) ++st_->resizes;
    }

    int32_t allocSlot() {
        if (freeHead_ != -1) {
            int32_t s = freeHead_;
            freeHead_ = entries_[s].next;
            return s;
        }
        entries_.push_back(Entry{});
        return (int32_t)entries_.size() - 1;
    }

public:
    HashTable() { buckets_.resize(8); for (size_t i = 0; i < 8; ++i) buckets_[i] = -1; }
    explicit HashTable(DsaStats* st) : HashTable() { st_ = st; buckets_.setStats(st); entries_.setStats(st); }
    void setStats(DsaStats* st) { st_ = st; buckets_.setStats(st); entries_.setStats(st); }

    size_t size() const { return live_; }
    bool empty() const { return live_ == 0; }
    size_t bucketCount() const { return buckets_.size(); }

    // Insert or update. Returns true if a NEW key was inserted.
    bool put(int32_t key, const V& value) {
        size_t b = bucketOf(key);
        for (int32_t i = buckets_[b]; i != -1; i = entries_[i].next) {
            if (st_) ++st_->probes;
            if (entries_[i].key == key) { entries_[i].value = value; return false; }
            if (st_) ++st_->collisions;
        }
        int32_t slot = allocSlot();
        Entry& e = entries_[slot];
        e.key = key; e.value = value; e.alive = true;
        e.next = buckets_[b];
        if (buckets_[b] != -1 && st_) ++st_->collisions;
        buckets_[b] = slot;
        ++live_;
        if (st_) ++st_->inserts;
        if (live_ * 4 >= buckets_.size() * 3) rehash(buckets_.size() * 2);
        return true;
    }

    bool get(int32_t key, V& out) const {
        if (st_) ++st_->lookups;
        size_t b = bucketOf(key);
        for (int32_t i = buckets_[b]; i != -1; i = entries_[i].next) {
            if (st_) ++st_->probes;
            if (entries_[i].key == key) { out = entries_[i].value; return true; }
            if (st_) ++st_->collisions;
        }
        return false;
    }

    bool contains(int32_t key) const { V tmp; return get(key, tmp); }

    bool erase(int32_t key) {
        size_t b = bucketOf(key);
        int32_t prev = -1;
        for (int32_t i = buckets_[b]; i != -1; i = entries_[i].next) {
            if (st_) ++st_->probes;
            if (entries_[i].key == key) {
                if (prev == -1) buckets_[b] = entries_[i].next;
                else entries_[prev].next = entries_[i].next;
                entries_[i].alive = false;
                entries_[i].next = freeHead_;
                freeHead_ = i;
                --live_;
                if (st_) ++st_->erases;
                // Shrink when load drops below 0.25 (never below 8 buckets) to give
                // memory back after bulk erases.
                if (buckets_.size() > 8 && live_ * 4 < buckets_.size())
                    rehash(buckets_.size() / 2);
                return true;
            }
            if (st_) ++st_->collisions;
            prev = i;
        }
        return false;
    }

    // Deterministic forward iterator over LIVE entries in dense slot order.
    class Iter {
        const DynArray<Entry>* arr_;
        size_t i_;
        void skip() { while (i_ < arr_->size() && !(*arr_)[i_].alive) ++i_; }
    public:
        Iter(const DynArray<Entry>* a, size_t i) : arr_(a), i_(i) { skip(); }
        const Entry& operator*() const { return (*arr_)[i_]; }
        Iter& operator++() { ++i_; skip(); return *this; }
        bool operator!=(const Iter& o) const { return i_ != o.i_; }
    };
    Iter begin() const { return Iter(&entries_, 0); }
    Iter end() const { return Iter(&entries_, entries_.size()); }
};

} // namespace tge
