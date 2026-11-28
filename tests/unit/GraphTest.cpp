// GraphTest.cpp — directed weighted graph vs a std::set edge model.
#include "doctest.h"
#include <set>
#include <utility>
#include "core/dsa/Graph.h"
#include "core/common/Rng.h"

TEST_CASE("Graph add/remove/has edge matches a set model over 100000 ops") {
    tge::Rng r(4242);
    tge::Graph g;
    const int N = 200;
    for (int i = 0; i < N; ++i) g.addNode();
    std::set<std::pair<int, int>> model;

    for (int i = 0; i < 100000; ++i) {
        int a = r.range(0, N - 1), b = r.range(0, N - 1);
        int op = r.range(0, 9);
        if (op < 5) {
            if (a != b && !g.hasEdge(a, b)) { g.addEdge(a, b, r.range(1, 9)); model.insert({a, b}); }
        } else if (op < 8) {
            bool got = g.removeEdge(a, b);
            bool want = model.erase({a, b}) > 0;
            CHECK(got == want);
        } else {
            CHECK(g.hasEdge(a, b) == (model.count({a, b}) > 0));
        }
        REQUIRE(g.edgeCount() == model.size());
    }
    // Degree consistency: out/in adjacency counts match the model.
    for (int n = 0; n < N; ++n) {
        size_t outDeg = 0, inDeg = 0;
        for (auto& e : model) { if (e.first == n) ++outDeg; if (e.second == n) ++inDeg; }
        CHECK(g.outEdges(n).size() == outDeg);
        CHECK(g.inEdges(n).size() == inDeg);
    }
}

TEST_CASE("Graph removeNode clears incident edges on both ends") {
    tge::Graph g;
    int a = g.addNode(), b = g.addNode(), c = g.addNode();
    g.addEdge(a, b);
    g.addEdge(b, c);
    g.addEdge(c, a);
    CHECK(g.edgeCount() == 3);
    g.removeNode(b);
    CHECK(!g.active(b));
    CHECK(g.edgeCount() == 1); // only c->a survives
    CHECK(g.hasEdge(c, a));
    CHECK(!g.hasEdge(a, b));
    CHECK(!g.hasEdge(b, c));
    CHECK(g.outEdges(a).size() == 0);
    CHECK(g.inEdges(c).size() == 0);
}

TEST_CASE("Graph reverse adjacency enables root tracing") {
    tge::Graph g;
    int root = g.addNode(), mid = g.addNode(), leaf = g.addNode();
    g.addEdge(root, mid);
    g.addEdge(mid, leaf);
    // walk backwards from leaf via in-edges
    int cur = leaf;
    tge::DynArray<int> path;
    while (true) {
        path.push_back(cur);
        if (g.inEdges(cur).empty()) break;
        cur = g.inEdges(cur).front().other;
    }
    REQUIRE(path.size() == 3);
    CHECK(path[0] == leaf);
    CHECK(path[2] == root);
}
