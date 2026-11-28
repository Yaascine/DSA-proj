// SortingTest.cpp — merge sort & heap sort vs std::sort on 100k random ints + stability.
#include "doctest.h"
#include <vector>
#include <algorithm>
#include "core/dsa/Sorting.h"
#include "core/dsa/DynArray.h"
#include "core/common/Rng.h"

namespace {
bool lessInt(const int& a, const int& b) { return a < b; }
struct Rec { int key; int idx; };
bool lessKey(const Rec& a, const Rec& b) { return a.key < b.key; }
}

TEST_CASE("mergeSort matches std::sort on 100000 random ints") {
    tge::Rng r(1);
    tge::DynArray<int> a;
    std::vector<int> v;
    for (int i = 0; i < 100000; ++i) { int x = r.range(-1000000, 1000000); a.push_back(x); v.push_back(x); }
    tge::mergeSort(a, lessInt);
    std::sort(v.begin(), v.end());
    REQUIRE(a.size() == v.size());
    for (size_t i = 0; i < v.size(); ++i) CHECK(a[i] == v[i]);
}

TEST_CASE("heapSort matches std::sort on 100000 random ints") {
    tge::Rng r(2);
    tge::DynArray<int> a;
    std::vector<int> v;
    for (int i = 0; i < 100000; ++i) { int x = r.range(-1000000, 1000000); a.push_back(x); v.push_back(x); }
    tge::heapSort(a, lessInt);
    std::sort(v.begin(), v.end());
    REQUIRE(a.size() == v.size());
    for (size_t i = 0; i < v.size(); ++i) CHECK(a[i] == v[i]);
}

TEST_CASE("mergeSort is stable (equal keys keep original order)") {
    tge::Rng r(3);
    tge::DynArray<Rec> a;
    for (int i = 0; i < 20000; ++i) a.push_back(Rec{r.range(0, 50), i});
    tge::mergeSort(a, lessKey);
    for (size_t i = 0; i + 1 < a.size(); ++i) {
        CHECK(a[i].key <= a[i + 1].key);
        if (a[i].key == a[i + 1].key) CHECK(a[i].idx < a[i + 1].idx);
    }
}

TEST_CASE("sorting empty and single-element arrays is safe") {
    tge::DynArray<int> e;
    tge::mergeSort(e, lessInt);
    tge::heapSort(e, lessInt);
    CHECK(e.empty());
    tge::DynArray<int> s; s.push_back(42);
    tge::mergeSort(s, lessInt);
    tge::heapSort(s, lessInt);
    CHECK(s[0] == 42);
}
