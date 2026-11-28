// AvlTree.h — hand-written self-balancing AVL binary search tree (K -> V, unique keys).
// Purpose: the wealth index (richest N, Gini coefficient) and any ordered ranking.
// Design: index-based node pool (no raw pointers). Each node stores height; after
//         insert/erase we rebalance with single/double rotations so height stays
//         O(log n). Rotations and comparisons are counted in DsaStats.
//         For the wealth index use a COMPOSITE key so equal wealth stays ordered
//         and unique: key = (int64_t(wealth) << 32) | uint32_t(humanId).
// Complexity: insert/erase/find O(log n); in-order walk O(n).
// Who uses it: P1 metrics (Gini), P3 wealth rankings.
#pragma once
#include <cstdint>
#include <cassert>
#include "DynArray.h"
#include "DsaStats.h"

namespace tge {

template <typename K, typename V>
class AvlTree {
public:
    using Handle = int32_t;
    static constexpr Handle NONE = -1;
    struct KV { K key{}; V value{}; };

private:
    struct Node {
        K key{};
        V value{};
        Handle left = NONE;
        Handle right = NONE;
        int height = 1;
        bool alive = false;
    };
    DynArray<Node> pool_;
    Handle root_ = NONE;
    Handle freeHead_ = NONE;
    size_t size_ = 0;
    DsaStats* st_ = nullptr;

    int height(Handle h) const { return h == NONE ? 0 : pool_[h].height; }
    void updateHeight(Handle h) {
        int hl = height(pool_[h].left), hr = height(pool_[h].right);
        pool_[h].height = 1 + (hl > hr ? hl : hr);
    }
    int balance(Handle h) const { return height(pool_[h].left) - height(pool_[h].right); }

    Handle rotateRight(Handle y) {
        Handle x = pool_[y].left;
        pool_[y].left = pool_[x].right;
        pool_[x].right = y;
        updateHeight(y); updateHeight(x);
        if (st_) ++st_->rotations;
        return x;
    }
    Handle rotateLeft(Handle y) {
        Handle x = pool_[y].right;
        pool_[y].right = pool_[x].left;
        pool_[x].left = y;
        updateHeight(y); updateHeight(x);
        if (st_) ++st_->rotations;
        return x;
    }
    Handle rebalance(Handle h) {
        updateHeight(h);
        int bf = balance(h);
        if (bf > 1) {
            if (balance(pool_[h].left) < 0) pool_[h].left = rotateLeft(pool_[h].left);
            return rotateRight(h);
        }
        if (bf < -1) {
            if (balance(pool_[h].right) > 0) pool_[h].right = rotateRight(pool_[h].right);
            return rotateLeft(h);
        }
        return h;
    }

    Handle allocNode(const K& k, const V& v) {
        Handle h;
        if (freeHead_ != NONE) { h = freeHead_; freeHead_ = pool_[h].right; }
        else { pool_.push_back(Node{}); h = (Handle)pool_.size() - 1; }
        Node& n = pool_[h];
        n.key = k; n.value = v; n.left = NONE; n.right = NONE; n.height = 1; n.alive = true;
        return h;
    }
    void freeNode(Handle h) {
        pool_[h].alive = false;
        pool_[h].right = freeHead_;
        freeHead_ = h;
    }

    Handle insertRec(Handle h, const K& k, const V& v, bool& inserted) {
        if (h == NONE) { ++size_; inserted = true; if (st_) ++st_->inserts; return allocNode(k, v); }
        if (st_) ++st_->comparisons;
        if (k < pool_[h].key) pool_[h].left = insertRec(pool_[h].left, k, v, inserted);
        else if (pool_[h].key < k) pool_[h].right = insertRec(pool_[h].right, k, v, inserted);
        else { pool_[h].value = v; return h; } // update existing
        return rebalance(h);
    }

    Handle minNode(Handle h) const { while (pool_[h].left != NONE) h = pool_[h].left; return h; }

    Handle eraseRec(Handle h, const K& k, bool& erased) {
        if (h == NONE) return NONE;
        if (st_) ++st_->comparisons;
        if (k < pool_[h].key) pool_[h].left = eraseRec(pool_[h].left, k, erased);
        else if (pool_[h].key < k) pool_[h].right = eraseRec(pool_[h].right, k, erased);
        else {
            erased = true;
            Handle l = pool_[h].left, r = pool_[h].right;
            if (l == NONE || r == NONE) {
                Handle child = (l != NONE) ? l : r;
                freeNode(h);
                --size_;
                if (st_) ++st_->erases;
                return child;
            }
            Handle s = minNode(r);
            pool_[h].key = pool_[s].key;
            pool_[h].value = pool_[s].value;
            bool dummy = false;
            pool_[h].right = eraseRec(r, pool_[s].key, dummy);
        }
        return rebalance(h);
    }

    void inOrderRec(Handle h, DynArray<KV>& out) const {
        if (h == NONE) return;
        inOrderRec(pool_[h].left, out);
        out.push_back(KV{pool_[h].key, pool_[h].value});
        inOrderRec(pool_[h].right, out);
    }
    void reverseRec(Handle h, DynArray<KV>& out) const {
        if (h == NONE) return;
        reverseRec(pool_[h].right, out);
        out.push_back(KV{pool_[h].key, pool_[h].value});
        reverseRec(pool_[h].left, out);
    }

public:
    AvlTree() = default;
    explicit AvlTree(DsaStats* st) : st_(st) { pool_.setStats(st); }
    void setStats(DsaStats* st) { st_ = st; pool_.setStats(st); }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    void insert(const K& k, const V& v) { bool ins = false; root_ = insertRec(root_, k, v, ins); }
    bool erase(const K& k) { bool er = false; root_ = eraseRec(root_, k, er); return er; }

    bool find(const K& k, V& out) const {
        if (st_) ++st_->lookups;
        Handle h = root_;
        while (h != NONE) {
            if (st_) ++st_->comparisons;
            if (k < pool_[h].key) h = pool_[h].left;
            else if (pool_[h].key < k) h = pool_[h].right;
            else { out = pool_[h].value; return true; }
        }
        return false;
    }
    bool contains(const K& k) const { V tmp; return find(k, tmp); }

    void inOrder(DynArray<KV>& out) const { inOrderRec(root_, out); }
    void reverseInOrder(DynArray<KV>& out) const { reverseRec(root_, out); }

    int treeHeight() const { return height(root_); }
};

} // namespace tge
