// DynArrayLifetimeTest.cpp — proves proper construction/destruction of elements.
#include "doctest.h"
#include <string>
#include "core/dsa/DynArray.h"

namespace {
struct Live {
    static int alive;
    Live() { ++alive; }
    ~Live() { --alive; }
    Live(const Live&) { ++alive; }
    Live(Live&&) noexcept { ++alive; }
    Live& operator=(const Live&) = default;
    Live& operator=(Live&&) noexcept = default;
};
int Live::alive = 0;
}

TEST_CASE("DynArray balances construction and destruction") {
    Live::alive = 0;
    {
        tge::DynArray<Live> a;
        for (int i = 0; i < 1000; ++i) a.push_back(Live{});
        CHECK(Live::alive == 1000);
        a.pop_back();          CHECK(Live::alive == 999);
        a.eraseAt(0);          CHECK(Live::alive == 998);
        a.swapRemoveAt(0);     CHECK(Live::alive == 997);
        a.clear();             CHECK(Live::alive == 0);
        for (int i = 0; i < 500; ++i) a.push_back(Live{});
        CHECK(Live::alive == 500);
        a.resize(100);         CHECK(Live::alive == 100);
        a.resize(300);         CHECK(Live::alive == 300);
        a.insertAt(0, Live{}); CHECK(Live::alive == 301);
    }
    CHECK(Live::alive == 0); // destructor released everything
}

TEST_CASE("DynArray handles non-trivial std::string elements") {
    tge::DynArray<std::string> a;
    for (int i = 0; i < 200; ++i) a.push_back(std::string("name") + std::to_string(i));
    CHECK(a[5] == "name5");
    a.eraseAt(0);
    a.pop_back();
    a.swapRemoveAt(3);
    a.clear();
    CHECK(a.empty());
}
