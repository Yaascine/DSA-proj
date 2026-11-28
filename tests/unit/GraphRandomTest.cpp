// GraphRandomTest.cpp — randomized node AND edge operations vs a reference model.
#include "doctest.h"
#include <set>
#include <utility>
#include "core/dsa/Graph.h"
#include "core/common/Rng.h"

TEST_CASE("Graph random node/edge ops match a model (incl. removeNode)") {
    tge::Rng r(2025);
    tge::Graph g;
    const int N = 80;
    for (int i = 0; i < N; ++i) g.addNode();
    std::set<int> active;
    for (int i = 0; i < N; ++i) active.insert(i);
    std::set<std::pair<int, int>> edges;

    for (int it = 0; it < 20000; ++it) {
        int op = r.range(0, 9);
        int a = r.range(0, N - 1), b = r.range(0, N - 1);
        if (op < 4) {
            if (active.count(a) && active.count(b) && a != b && !edges.count({a, b})) {
                g.addEdge(a, b, r.range(1, 9));
                edges.insert({a, b});
            }
        } else if (op < 7) {
            bool got = g.removeEdge(a, b);
            bool want = edges.erase({a, b}) > 0;
            CHECK(got == want);
        } else if (op < 9) {
            CHECK(g.hasEdge(a, b) == (edges.count({a, b}) > 0));
        } else {
            if (active.count(a)) {
                g.removeNode(a);
                active.erase(a);
                for (auto e = edges.begin(); e != edges.end();) {
                    if (e->first == a || e->second == a) e = edges.erase(e);
                    else ++e;
                }
            }
        }
        REQUIRE(g.edgeCount() == edges.size());
        REQUIRE(g.nodeCount() == active.size());
    }
    for (int n = 0; n < N; ++n) CHECK(g.active(n) == (active.count(n) > 0));
}
