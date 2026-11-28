// Sorting.h — hand-written merge sort and heap sort over our DynArray.
// Purpose: rankings and reports (richest lists, event ordering, diagnostics).
// Design: both take a plain comparator function pointer `less(a,b)` (true if a
//         must come before b). Merge sort is STABLE and uses a scratch buffer;
//         heap sort is in-place. Comparisons and swaps/moves are counted.
// Complexity: merge sort O(n log n) time, O(n) extra space; heap sort O(n log n)
//             time, O(1) extra space.
// Who uses it: P1 reports/metrics; P3/P4 rankings.
#pragma once
#include <cstddef>
#include <utility>
#include "DynArray.h"
#include "DsaStats.h"

namespace tge {

template <typename T>
using LessFn = bool (*)(const T&, const T&);

namespace sorting_detail {
    template <typename T>
    void mergeRun(DynArray<T>& a, DynArray<T>& tmp, size_t lo, size_t mid, size_t hi,
                  LessFn<T> less, DsaStats* st) {
        size_t i = lo, j = mid, k = lo;
        while (i < mid && j < hi) {
            if (st) ++st->comparisons;
            // <= keeps merge sort STABLE (take from left on ties).
            if (!less(a[j], a[i])) tmp[k++] = std::move(a[i++]);
            else tmp[k++] = std::move(a[j++]);
        }
        while (i < mid) tmp[k++] = std::move(a[i++]);
        while (j < hi) tmp[k++] = std::move(a[j++]);
        for (size_t x = lo; x < hi; ++x) { a[x] = std::move(tmp[x]); if (st) ++st->swaps; }
    }
    template <typename T>
    void mergeRec(DynArray<T>& a, DynArray<T>& tmp, size_t lo, size_t hi,
                  LessFn<T> less, DsaStats* st) {
        if (hi - lo <= 1) return;
        size_t mid = lo + (hi - lo) / 2;
        mergeRec(a, tmp, lo, mid, less, st);
        mergeRec(a, tmp, mid, hi, less, st);
        mergeRun(a, tmp, lo, mid, hi, less, st);
    }
}

template <typename T>
void mergeSort(DynArray<T>& a, LessFn<T> less, DsaStats* st = nullptr) {
    if (a.size() <= 1) return;
    DynArray<T> tmp;
    tmp.resize(a.size());
    sorting_detail::mergeRec(a, tmp, 0, a.size(), less, st);
}

template <typename T>
void heapSort(DynArray<T>& a, LessFn<T> less, DsaStats* st = nullptr) {
    size_t n = a.size();
    if (n <= 1) return;
    // siftDown builds/repairs a MAX-heap under `less` (largest at root).
    auto siftDown = [&](size_t start, size_t end) {
        size_t root = start;
        for (;;) {
            size_t child = 2 * root + 1;
            if (child >= end) break;
            if (st) ++st->comparisons;
            if (child + 1 < end && less(a[child], a[child + 1])) ++child;
            if (st) ++st->comparisons;
            if (!less(a[root], a[child])) break;
            std::swap(a[root], a[child]);
            if (st) ++st->swaps;
            root = child;
        }
    };
    for (size_t start = n / 2; start-- > 0;) siftDown(start, n);
    for (size_t end = n - 1; end > 0; --end) {
        std::swap(a[0], a[end]);
        if (st) ++st->swaps;
        siftDown(0, end);
    }
}

} // namespace tge
