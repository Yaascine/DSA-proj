// WealthMetrics.h — Gini coefficient from the AVL wealth index (METRICS ONLY).
// Purpose: inequality statistic for the diagnostics panel. Floats are allowed here
//          because this is a metric, never simulation state.
// Formula (ascending wealth x[0..n-1]):
//   G = (2 * sum_{i=1..n} i * x[i-1]) / (n * sum(x)) - (n + 1) / n
// Complexity: O(n) walk of the AVL in ascending order.
// Who uses it: P1 metrics / P4 diagnostics.
#pragma once
#include "core/dsa/AvlTree.h"
#include "core/dsa/DynArray.h"
#include "core/common/Types.h"

namespace tge {

// Gini from an already-ascending array of wealth values.
inline double giniOfSorted(const DynArray<int>& x) {
    size_t n = x.size();
    if (n == 0) return 0.0;
    double total = 0.0, weighted = 0.0;
    for (size_t i = 0; i < n; ++i) {
        total += (double)x[i];
        weighted += (double)(i + 1) * (double)x[i];
    }
    if (total <= 0.0) return 0.0;
    return (2.0 * weighted) / ((double)n * total) - ((double)n + 1.0) / (double)n;
}

// Wealth index key convention: (int64_t(wealth) << 32) | uint32_t(humanId).
inline double giniFromWealthIndex(const AvlTree<int64_t, HumanId>& wealth) {
    DynArray<AvlTree<int64_t, HumanId>::KV> out;
    wealth.inOrder(out);
    DynArray<int> xs;
    for (size_t i = 0; i < out.size(); ++i) xs.push_back((int)(out[i].key >> 32));
    return giniOfSorted(xs);
}

} // namespace tge
