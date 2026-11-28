// History.cpp — event log, explicit cause graph, reverse/forward BFS, per-human index.
#include "core/history/History.h"
#include "core/dsa/Queue.h"
#include "core/dsa/Sorting.h"
#include <cassert>

namespace tge {

namespace {
bool lessEventId(const EventId& a, const EventId& b) { return a < b; }
}

History::History() { causeGraph_.setStats(&stats_); }

void History::ensureHuman(HumanId id) {
    if (id < 0) return;
    if ((size_t)id >= perHuman_.size()) {
        size_t old = perHuman_.size();
        perHuman_.resize((size_t)id + 1);
        for (size_t i = old; i < perHuman_.size(); ++i) perHuman_[i].setStats(&stats_);
    }
}

EventId History::record(EventType type, Tick tick, std::vector<HumanId> actors, TileCoord where,
                        int param, std::vector<EventId> causes, uint32_t flags) {
    EventId id = (EventId)events_.size();
    EventRecord rec;
    rec.id = id; rec.tick = tick; rec.type = type; rec.where = where;
    rec.param = param; rec.flags = flags;
    for (HumanId a : actors) rec.actors.push_back(a);
    for (EventId c : causes) rec.causes.push_back(c);
    events_.push_back(std::move(rec));

    int32_t node = causeGraph_.addNode();
    assert(node == id);
    // HARD RULE: edges only from explicitly-passed causeIds. Never infer from timing.
    for (EventId c : causes) {
        if (c >= 0 && (size_t)c < events_.size() - 1) causeGraph_.addEdge(c, id);
    }
    for (HumanId a : actors) {
        ensureHuman(a);
        if (a >= 0) perHuman_[a].pushBack(id);
    }
    ++stats_.inserts;
    return id;
}

DynArray<EventId> History::traceRoots(EventId id) const {
    DynArray<EventId> roots;
    if (id < 0 || (size_t)id >= events_.size()) return roots;
    DynArray<bool> visited;
    visited.resize(events_.size());
    Queue<EventId> q;
    q.push(id);
    visited[id] = true;
    while (!q.empty()) {
        EventId cur = q.pop();
        const auto& in = causeGraph_.inEdges(cur);
        if (in.empty()) { roots.push_back(cur); continue; }
        for (const Graph::Edge& e : in) {
            if (!visited[e.other]) { visited[e.other] = true; q.push(e.other); }
        }
    }
    mergeSort(roots, lessEventId); // canonical ascending order
    return roots;
}

DynArray<EventId> History::chain(EventId from, EventId to) const {
    DynArray<EventId> path;
    if (from < 0 || to < 0 || (size_t)from >= events_.size() || (size_t)to >= events_.size())
        return path;
    if (from == to) { path.push_back(from); return path; }
    DynArray<EventId> parent;
    parent.resize(events_.size());
    for (size_t i = 0; i < parent.size(); ++i) parent[i] = -1;
    DynArray<bool> visited;
    visited.resize(events_.size());
    Queue<EventId> q;
    q.push(from);
    visited[from] = true;
    bool found = false;
    while (!q.empty() && !found) {
        EventId cur = q.pop();
        for (const Graph::Edge& e : causeGraph_.outEdges(cur)) {
            EventId nx = e.other;
            if (visited[nx]) continue;
            visited[nx] = true;
            parent[nx] = cur;
            if (nx == to) { found = true; break; }
            q.push(nx);
        }
    }
    if (!found) return path;
    for (EventId cur = to; cur != -1; cur = parent[cur]) path.push_back(cur);
    // reverse in place (path is to -> from)
    for (size_t i = 0, j = path.size() - 1; i < j; ++i, --j) {
        EventId t = path[i]; path[i] = path[j]; path[j] = t;
    }
    return path;
}

const LinkedList<EventId>& History::eventsFor(HumanId id) const {
    static const LinkedList<EventId> empty;
    if (id < 0 || (size_t)id >= perHuman_.size()) return empty;
    return perHuman_[id];
}

void History::hashInto(Fnv& f) const {
    f.add64((uint64_t)events_.size());
    for (size_t i = 0; i < events_.size(); ++i) {
        const EventRecord& e = events_[i];
        f.add32((uint32_t)e.id);
        f.add64((uint64_t)e.tick);
        f.add8((uint8_t)e.type);
        f.add16((uint16_t)e.where.x); f.add16((uint16_t)e.where.y);
        f.add32((uint32_t)e.param);
        f.add32(e.flags);
        f.add64((uint64_t)e.actors.size());
        for (size_t a = 0; a < e.actors.size(); ++a) f.add32((uint32_t)e.actors[a]);
        f.add64((uint64_t)e.causes.size());
        for (size_t c = 0; c < e.causes.size(); ++c) f.add32((uint32_t)e.causes[c]);
    }
}

void History::restore(const DynArray<EventRecord>& src) {
    clear();
    for (size_t i = 0; i < src.size(); ++i) {
        const EventRecord& e = src[i];
        std::vector<HumanId> actors;
        for (size_t a = 0; a < e.actors.size(); ++a) actors.push_back(e.actors[a]);
        std::vector<EventId> causes;
        for (size_t c = 0; c < e.causes.size(); ++c) causes.push_back(e.causes[c]);
        record(e.type, e.tick, actors, e.where, e.param, causes, e.flags);
    }
}

void History::clear() {
    events_.clear();
    causeGraph_ = Graph();
    causeGraph_.setStats(&stats_);
    perHuman_.clear();
    stats_.reset();
}

} // namespace tge
