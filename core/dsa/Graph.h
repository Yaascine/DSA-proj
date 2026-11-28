// Graph.h — hand-written directed, weighted graph with adjacency LISTS.
// Purpose: the history cause graph (cause -> effect) and P3 relationship graphs.
// Design: nodes get stable sequential int32 ids (never reused, so entities can
//         reference each other by id). Each node keeps TWO adjacency lists built
//         from our LinkedList: out-edges and in-edges. Keeping in-edges makes
//         reverse traversals (traceRoots via reverse BFS) cheap and explicit.
//         removeNode clears every incident edge on both ends.
// Complexity: addNode O(1); addEdge O(1); removeEdge O(degree);
//             removeNode O(degree); neighbor iteration O(degree).
// Who uses it: P1 history cause graph; P3 relationships.
#pragma once
#include <cstdint>
#include <cassert>
#include "DynArray.h"
#include "LinkedList.h"
#include "DsaStats.h"

namespace tge {

class Graph {
public:
    struct Edge { int32_t other = -1; int32_t weight = 1; };

private:
    struct Node {
        bool active = false;
        LinkedList<Edge> out;  // other = destination
        LinkedList<Edge> in;   // other = source
    };
    DynArray<Node> nodes_;
    size_t activeNodes_ = 0;
    size_t edgeCount_ = 0;
    DsaStats* st_ = nullptr;

    bool valid(int32_t id) const { return id >= 0 && (size_t)id < nodes_.size() && nodes_[id].active; }

    // Remove the edge entry pointing at `other` from list `l`. Returns true if found.
    static bool unlink(LinkedList<Edge>& l, int32_t other) {
        for (auto it = l.begin(); it != l.end(); ++it) {
            if ((*it).other == other) { l.remove(it.handle()); return true; }
        }
        return false;
    }

public:
    Graph() = default;
    explicit Graph(DsaStats* st) : st_(st) { nodes_.setStats(st); }
    void setStats(DsaStats* st) { st_ = st; nodes_.setStats(st); }

    int32_t addNode() {
        nodes_.push_back(Node{});
        int32_t id = (int32_t)nodes_.size() - 1;
        nodes_[id].active = true;
        nodes_[id].out.setStats(st_);
        nodes_[id].in.setStats(st_);
        ++activeNodes_;
        if (st_) ++st_->inserts;
        return id;
    }
    void removeNode(int32_t id) {
        if (!valid(id)) return;
        Node& n = nodes_[id];
        // Clear out-edges: drop the matching in-entry on each destination.
        while (!n.out.empty()) {
            int32_t dst = n.out.front().other;
            unlink(nodes_[dst].in, id);
            n.out.remove(n.out.head());
            --edgeCount_;
        }
        // Clear in-edges: drop the matching out-entry on each source.
        while (!n.in.empty()) {
            int32_t src = n.in.front().other;
            unlink(nodes_[src].out, id);
            n.in.remove(n.in.head());
            --edgeCount_;
        }
        n.active = false;
        --activeNodes_;
        if (st_) ++st_->erases;
    }

    void addEdge(int32_t from, int32_t to, int32_t weight = 1) {
        assert(valid(from) && valid(to));
        nodes_[from].out.pushBack(Edge{to, weight});
        nodes_[to].in.pushBack(Edge{from, weight});
        ++edgeCount_;
        if (st_) ++st_->inserts;
    }
    bool removeEdge(int32_t from, int32_t to) {
        if (!valid(from) || !valid(to)) return false;
        bool a = unlink(nodes_[from].out, to);
        bool b = unlink(nodes_[to].in, from);
        if (a && b) { --edgeCount_; if (st_) ++st_->erases; return true; }
        return false;
    }
    bool hasEdge(int32_t from, int32_t to) const {
        if (!valid(from) || !valid(to)) return false;
        for (const Edge& e : nodes_[from].out) if (e.other == to) return true;
        return false;
    }

    bool active(int32_t id) const { return valid(id); }
    size_t nodeCount() const { return activeNodes_; }
    size_t edgeCount() const { return edgeCount_; }
    size_t nodeSlots() const { return nodes_.size(); }
    const LinkedList<Edge>& outEdges(int32_t id) const { assert(valid(id)); return nodes_[id].out; }
    const LinkedList<Edge>& inEdges(int32_t id) const { assert(valid(id)); return nodes_[id].in; }
};

} // namespace tge
